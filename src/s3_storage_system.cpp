#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <libs3.h>

#include "s3_storage_system.h"

#ifndef SLEEP_UNITS_PER_SECOND
#define SLEEP_UNITS_PER_SECOND 1
#endif

using namespace std;

// Helpful links:
// https://docs.ceph.com/en/latest/radosgw/s3/csharp/

// https://docs.ceph.com/en/latest/radosgw/s3/cpp/
//
static int retriesG = 3;
static S3Status statusG = S3StatusOK;
static int showResponsePropertiesG = 0;
static char errorDetailsG[4096] = { 0 };

struct _growbuffer
{
    // The total number of bytes, and the start byte
    int size;
    // The start byte
    int start;
    // The blocks
    char data[64 * 1024];
    struct _growbuffer *prev, *next;
};

typedef struct _growbuffer growbuffer;

struct _put_object_callback_data
{
    FILE* infile;
    growbuffer* gb;
    uint64_t contentLength;
    uint64_t originalContentLength;
    int noStatus;
};

typedef struct _put_object_callback_data put_object_callback_data;

struct _list_service_data
{
    int headerPrinted;
    int allDetails;
};

typedef struct _list_service_data list_service_data;

struct _list_bucket_callback_data
{
    int isTruncated;
    char nextMarker[1024];
    int keyCount;
    int allDetails;
};

typedef struct _list_bucket_callback_data list_bucket_callback_data;

struct _get_object_callback_data
{
   FILE* outfile;
   int64_t bytes_retrieved;
};

typedef struct _get_object_callback_data get_object_callback_data;

//*****************************************************************************

// returns nonzero on success, zero on out of memory
static int growbuffer_append(growbuffer **gb, const char *data, int dataLen)
{
    while (dataLen) {
        growbuffer *buf = *gb ? (*gb)->prev : 0;
        if (!buf || (buf->size == sizeof(buf->data))) {
            buf = (growbuffer *) malloc(sizeof(growbuffer));
            if (!buf) {
                return 0;
            }
            buf->size = 0;
            buf->start = 0;
            if (*gb && (*gb)->prev) {
                buf->prev = (*gb)->prev;
                buf->next = *gb;
                (*gb)->prev->next = buf;
                (*gb)->prev = buf;
            }
            else {
                buf->prev = buf->next = buf;
                *gb = buf;
            }
        }

        int toCopy = (sizeof(buf->data) - buf->size);
        if (toCopy > dataLen) {
            toCopy = dataLen;
        }

        memcpy(&(buf->data[buf->size]), data, toCopy);
        
        buf->size += toCopy, data += toCopy, dataLen -= toCopy;
    }

    return 1;
}

//*****************************************************************************

static void growbuffer_read(growbuffer **gb, int amt, int *amtReturn, 
                            char *buffer)
{
    *amtReturn = 0;

    growbuffer *buf = *gb;

    if (!buf) {
        return;
    }

    *amtReturn = (buf->size > amt) ? amt : buf->size;

    memcpy(buffer, &(buf->data[buf->start]), *amtReturn);
    
    buf->start += *amtReturn, buf->size -= *amtReturn;

    if (buf->size == 0) {
        if (buf->next == buf) {
            *gb = 0;
        }
        else {
            *gb = buf->next;
            buf->prev->next = buf->next;
            buf->next->prev = buf->prev;
        }
        free(buf);
    }
}

//*****************************************************************************

static void growbuffer_destroy(growbuffer *gb)
{
    growbuffer *start = gb;

    while (gb) {
        growbuffer *next = gb->next;
        free(gb);
        gb = (next == start) ? 0 : next;
    }
}

//*****************************************************************************

static int should_retry(void)
{
    if (retriesG--) {
        // Sleep before next retry; start out with a 1 second sleep
        static int retrySleepInterval = 1 * SLEEP_UNITS_PER_SECOND;
        sleep(retrySleepInterval);
        // Next sleep 1 second longer
        retrySleepInterval++;
        return 1;
    }

    return 0;
}

//*****************************************************************************

static void printListBucketHeader(int allDetails)
{
    printf("%-50s  %-20s  %-5s", 
           "                       Key", 
           "   Last Modified", "Size");
    if (allDetails) {
        printf("  %-34s  %-64s  %-12s", 
               "               ETag", 
               "                            Owner ID",
               "Display Name");
    }
    printf("\n");
    printf("--------------------------------------------------  "
           "--------------------  -----");
    if (allDetails) {
        printf("  ----------------------------------  "
               "-------------------------------------------------"
               "---------------  ------------");
    }
    printf("\n");
}

//*****************************************************************************

static void printListServiceHeader(int allDetails)
{
    printf("%-56s  %-20s", "                         Bucket",
           "      Created");
    if (allDetails) {
        printf("  %-64s  %-12s", 
               "                            Owner ID",
               "Display Name");
    }
    printf("\n");
    printf("--------------------------------------------------------  "
           "--------------------");
    if (allDetails) {
        printf("  -------------------------------------------------"
               "---------------  ------------");
    }
    printf("\n");
}

//*****************************************************************************

static S3Status listServiceCallback(const char *ownerId, 
                                    const char *ownerDisplayName,
                                    const char *bucketName,
                                    int64_t creationDate,
                                    void *callbackData)
{
    list_service_data *data = (list_service_data *) callbackData;

    if (!data->headerPrinted) {
        data->headerPrinted = 1;
        printListServiceHeader(data->allDetails);
    }

    char timebuf[256];
    if (creationDate >= 0) {
        time_t t = (time_t) creationDate;
        strftime(timebuf, sizeof(timebuf), "%Y-%m-%dT%H:%M:%SZ", gmtime(&t));
    }
    else {
        timebuf[0] = 0;
    }

    printf("%-56s  %-20s", bucketName, timebuf);
    if (data->allDetails) {
        printf("  %-64s  %-12s", ownerId ? ownerId : "", 
               ownerDisplayName ? ownerDisplayName : "");
    }
    printf("\n");

    return S3StatusOK;
}

//*****************************************************************************

static S3Status listBucketCallback(int isTruncated,
                                   const char *nextMarker,
                                   int contentsCount, 
                                   const S3ListBucketContent *contents,
                                   int commonPrefixesCount,
                                   const char **commonPrefixes,
                                   void *callbackData)
{
    list_bucket_callback_data *data = 
        (list_bucket_callback_data *) callbackData;

    data->isTruncated = isTruncated;
    // This is tricky.  S3 doesn't return the NextMarker if there is no
    // delimiter.  Why, I don't know, since it's still useful for paging
    // through results.  We want NextMarker to be the last content in the
    // list, so set it to that if necessary.
    if ((!nextMarker || !nextMarker[0]) && contentsCount) {
        nextMarker = contents[contentsCount - 1].key;
    }
    if (nextMarker) {
        snprintf(data->nextMarker, sizeof(data->nextMarker), "%s", 
                 nextMarker);
    }
    else {
        data->nextMarker[0] = 0;
    }
    
    if (contentsCount && !data->keyCount) {
        printListBucketHeader(data->allDetails);
    }

    for (int i = 0; i < contentsCount; i++) {
        const S3ListBucketContent *content = &(contents[i]);
        char timebuf[256];
        if (0) {
            time_t t = (time_t) content->lastModified;
            strftime(timebuf, sizeof(timebuf), "%Y-%m-%dT%H:%M:%SZ",
                     gmtime(&t));
            printf("\nKey: %s\n", content->key);
            printf("Last Modified: %s\n", timebuf);
            printf("ETag: %s\n", content->eTag);
            printf("Size: %llu\n", (unsigned long long) content->size);
            if (content->ownerId) {
                printf("Owner ID: %s\n", content->ownerId);
            }
            if (content->ownerDisplayName) {
                printf("Owner Display Name: %s\n", content->ownerDisplayName);
            }
        }
        else {
            time_t t = (time_t) content->lastModified;
            strftime(timebuf, sizeof(timebuf), "%Y-%m-%dT%H:%M:%SZ", 
                     gmtime(&t));
            char sizebuf[16];
            if (content->size < 100000) {
                sprintf(sizebuf, "%5llu", (unsigned long long) content->size);
            }
            else if (content->size < (1024 * 1024)) {
                sprintf(sizebuf, "%4lluK", 
                        ((unsigned long long) content->size) / 1024ULL);
            }
            else if (content->size < (10 * 1024 * 1024)) {
                float f = content->size;
                f /= (1024 * 1024);
                sprintf(sizebuf, "%1.2fM", f);
            }
            else if (content->size < (1024 * 1024 * 1024)) {
                sprintf(sizebuf, "%4lluM", 
                        ((unsigned long long) content->size) / 
                        (1024ULL * 1024ULL));
            }
            else {
                float f = (content->size / 1024);
                f /= (1024 * 1024);
                sprintf(sizebuf, "%1.2fG", f);
            }
            printf("%-50s  %s  %s", content->key, timebuf, sizebuf);
            if (data->allDetails) {
                printf("  %-34s  %-64s  %-12s",
                       content->eTag, 
                       content->ownerId ? content->ownerId : "",
                       content->ownerDisplayName ? 
                       content->ownerDisplayName : "");
            }
            printf("\n");
        }
    }

    data->keyCount += contentsCount;

    for (int i = 0; i < commonPrefixesCount; i++) {
        printf("\nCommon Prefix: %s\n", commonPrefixes[i]);
    }

    return S3StatusOK;
}

//*****************************************************************************

static S3Status responsePropertiesCallback
    (const S3ResponseProperties *properties, void *callbackData)
{
   PropertySet* props = NULL;
    
   if (callbackData != NULL) {
      props = static_cast<PropertySet*>(callbackData);
   }

   if (!showResponsePropertiesG || (props == NULL)) {
      return S3StatusOK;
   }

   props->add("Content-Type", new StrPropertyValue(properties->contentType));
   props->add("Request-Id", new StrPropertyValue(properties->requestId));
   props->add("Request-Id-2", new StrPropertyValue(properties->requestId2));
   
   if (properties->contentLength > 0) {
      props->add("Content-Length", new ULongPropertyValue(properties->contentLength));
   }

   props->add("Server", new StrPropertyValue(properties->server));
   props->add("ETag", new StrPropertyValue(properties->eTag));

   if (properties->lastModified > 0) {
      char timebuf[256];
      time_t t = (time_t) properties->lastModified;
      // gmtime is not thread-safe but we don't care here.
      strftime(timebuf, sizeof(timebuf), "%Y-%m-%dT%H:%M:%SZ", gmtime(&t));
      props->add("Last-Modified", new StrPropertyValue(timebuf));
   }
   
   for (int i = 0; i < properties->metaDataCount; i++) {
      char propName[256];
      memset(propName, 0, sizeof(propName));
      snprintf(propName, 256, "x-amz-meta-%s", properties->metaData[i].name);
      props->add(propName, new StrPropertyValue(properties->metaData[i].value));
   }

   return S3StatusOK;
}

//*****************************************************************************

static int putObjectDataCallback(int bufferSize, char *buffer,
                                 void *callbackData)
{
    put_object_callback_data *data = 
        (put_object_callback_data *) callbackData;
    
    int ret = 0;

    if (data->contentLength) {
        int toRead = ((data->contentLength > (unsigned) bufferSize) ?
                      (unsigned) bufferSize : data->contentLength);
        if (data->gb) {
            growbuffer_read(&(data->gb), toRead, &ret, buffer);
        }
        else if (data->infile) {
            ret = fread(buffer, 1, toRead, data->infile);
        }
    }

    data->contentLength -= ret;

    if (data->contentLength && !data->noStatus) {
        // Avoid a weird bug in MingW, which won't print the second integer
        // value properly when it's in the same call, so print separately
        printf("%llu bytes remaining ", 
               (unsigned long long) data->contentLength);
        printf("(%d%% complete) ...\n",
               (int) (((data->originalContentLength - 
                        data->contentLength) * 100) /
                      data->originalContentLength));
    }

    return ret;
}

//*****************************************************************************

static void responseCompleteCallback(S3Status status,
                                     const S3ErrorDetails *error, 
                                     void *callbackData)
{
    (void) callbackData;

    statusG = status;
    // Compose the error details message now, although we might not use it.
    // Can't just save a pointer to [error] since it's not guaranteed to last
    // beyond this callback
    int len = 0;
    if (error && error->message) {
        len += snprintf(&(errorDetailsG[len]), sizeof(errorDetailsG) - len,
                        "  Message: %s\n", error->message);
    }
    if (error && error->resource) {
        len += snprintf(&(errorDetailsG[len]), sizeof(errorDetailsG) - len,
                        "  Resource: %s\n", error->resource);
    }
    if (error && error->furtherDetails) {
        len += snprintf(&(errorDetailsG[len]), sizeof(errorDetailsG) - len,
                        "  Further Details: %s\n", error->furtherDetails);
    }
    if (error && error->extraDetailsCount) {
        len += snprintf(&(errorDetailsG[len]), sizeof(errorDetailsG) - len,
                        "%s", "  Extra Details:\n");
        int i;
        for (i = 0; i < error->extraDetailsCount; i++) {
            len += snprintf(&(errorDetailsG[len]), 
                            sizeof(errorDetailsG) - len, "    %s: %s\n", 
                            error->extraDetails[i].name,
                            error->extraDetails[i].value);
        }
    }
}

//*****************************************************************************

static S3Status getObjectDataCallback(int bufferSize,
                                      const char* buffer,
                                      void* callbackData)
{
   get_object_callback_data* data = (get_object_callback_data*) callbackData;

   size_t wrote = fwrite(buffer, 1, bufferSize, data->outfile);
   if (wrote > 0) {
      data->bytes_retrieved += wrote;
   }

   return ((wrote < (size_t) bufferSize) ?
           S3StatusAbortedByCallback : S3StatusOK);
}

//*****************************************************************************
//*****************************************************************************

S3StorageSystem::S3StorageSystem(const string& access_key,
                                 const string& secret_key,
                                 const string& container_prefix,
                                 bool debug) :
   StorageSystem("S3", debug),
   debug_mode(true), //debug),
   connected(false),
   aws_access_key(access_key),
   aws_secret_key(secret_key),
   s3_host("s3.us-central-1.wasabisys.com"),
   s3_protocol(S3ProtocolHTTPS),
   s3_uri_style(S3UriStyleVirtualHost)
{

   if (debug_mode) {
      printf("Using access_key=%s, secret_key=%s\n",
             aws_access_key.c_str(), aws_secret_key.c_str());
   }
   if (container_prefix.length() > 0) {
      if (debug_mode) {
         printf("using container_prefix=%s\n", container_prefix.c_str());
      }
   }
}

//*****************************************************************************

S3StorageSystem::~S3StorageSystem() {
   exit();
}

//*****************************************************************************

bool S3StorageSystem::enter() {
   if (debug_mode) {
      printf("S3StorageSystem.enter\n");
      printf("attempting to connect to S3\n");
   }

   S3Status status = S3_initialize("s3",
                                   S3_INIT_ALL,
                                   s3_host.c_str());
   if (status == S3StatusOK) {
      authenticated = true;
      connected = true;
      //list_containers = list_account_containers();
      return true;
   } else {
      return false;
   }
}

//*****************************************************************************

void S3StorageSystem::exit() {
   if (debug_mode) {
      printf("S3StorageSystem.exit\n");
   }

   S3_deinitialize();

   connected = false;
   authenticated = false;
}

//*****************************************************************************

vector<string> S3StorageSystem::list_account_containers() {
   if (debug_mode) {
      printf("list_account_containers\n");
   }

   list_service_data data;

   S3ListServiceHandler listServiceHandler =
   {
      { &responsePropertiesCallback, &responseCompleteCallback },
      &listServiceCallback
   };

   do {
      S3_list_service(s3_protocol,             // S3Protocol protocol
                      aws_access_key.c_str(),  // const char *accessKeyId
                      aws_secret_key.c_str(),  // const char *secretAccessKey
                      s3_host.c_str(),         // const char *hostName
                      NULL,                    // S3RequestContext *requestContext
                      &listServiceHandler,     // const S3ListServiceHandler *handler
                      &data);                  // void *callbackData
   } while (S3_status_is_retryable(statusG) && should_retry());

   return vector<string>();
}

//*****************************************************************************

bool S3StorageSystem::create_container(const string& container_name) {
   if (debug_mode) {
      printf("create_container: %s\n", container_name.c_str());
   }

   bool container_created = false;

   const char *locationConstraint = 0;
   S3CannedAcl cannedAcl = S3CannedAclPrivate;

   S3ResponseHandler responseHandler =
   {
      &responsePropertiesCallback, &responseCompleteCallback
   };

   do {
      S3_create_bucket(s3_protocol,             // S3Protocol protocol
                       aws_access_key.c_str(),  // const char *accessKeyId
                       aws_secret_key.c_str(),  // const char *secretAccessKey
                       s3_host.c_str(),         // const char *hostName
                       container_name.c_str(),  // const char *bucketName
                       cannedAcl,               // S3CannedAcl cannedAcl
                       locationConstraint,      // const char *locationConstraint
                       NULL,                    // S3RequestContext *requestContext
                       &responseHandler,        // const S3ResponseHandler *handler
                       NULL);                   // void *callbackData
   } while (S3_status_is_retryable(statusG) & should_retry());
   
   if (statusG == S3StatusOK) {
      container_created = true;
   }
   
   return container_created;
}

//*****************************************************************************

bool S3StorageSystem::delete_container(const string& container_name) {
   if (debug_mode) {
      printf("delete_container: %s\n", container_name.c_str());
   }

   bool container_deleted = false;

   S3ResponseHandler responseHandler =
   {
      &responsePropertiesCallback, &responseCompleteCallback
   };

   do {
      S3_delete_bucket(s3_protocol,             // S3Protocol protocol
                       s3_uri_style,            // S3UriStyle uriStyle
                       aws_access_key.c_str(),  // const char *accessKeyId
                       aws_secret_key.c_str(),  // const char *secretAccessKey
                       s3_host.c_str(),         // const char *hostName
                       container_name.c_str(),  // const char *bucketName
                       NULL,                    // S3RequestContext *requestContext
                       &responseHandler,        // const S3ResponseHandler *handler
                       NULL);                   // void *callbackData
   } while (S3_status_is_retryable(statusG) && should_retry());
   
   if (statusG == S3StatusOK) {
      container_deleted = true;
   }

   return container_deleted;
}

//*****************************************************************************

vector<string> S3StorageSystem::list_container_contents(const string& container_name) {
   if (debug_mode) {
      printf("list_container_contents: %s\n", container_name.c_str());
   }

   vector<string> list_objects;

   S3BucketContext bucketContext;
   populateBucketContext(bucketContext, container_name);
   
   S3ListBucketHandler listBucketHandler =
   {
      { &responsePropertiesCallback, &responseCompleteCallback },
      &listBucketCallback
   };

   list_bucket_callback_data data;
   const char *marker = 0;
   int maxkeys = 0;
   int allDetails = 0;
   
   snprintf(data.nextMarker, sizeof(data.nextMarker), "%s", marker);
   data.keyCount = 0;
   data.allDetails = allDetails;

   do {
      data.isTruncated = 0;
      do {
         S3_list_bucket(&bucketContext,      // const S3BucketContext *bucketContext
                        0,                   // const char *prefix
                        data.nextMarker,     // const char *marker
                        0,                   // const char *delimiter
                        0,                   // int maxkeys
                        NULL,                // S3RequestContext*
                        &listBucketHandler,  // const S3ListBucketHandler *handler
                        &data);              // void *callbackData
      } while (S3_status_is_retryable(statusG) && should_retry());
      
      if (statusG != S3StatusOK) {
         break;
      }
   } while (data.isTruncated && (!maxkeys || (data.keyCount < maxkeys)));
   
   return list_objects;
}

//*****************************************************************************

bool S3StorageSystem::get_object_metadata(const string& container_name,
                                          const string& object_name,
                                          PropertySet& properties) {
   if (debug_mode) {
      printf("get_object_metadata: container=%s, object=%s\n",
             container_name.c_str(), object_name.c_str());
   }
   
   bool success = false;

   /*
   S3BucketContext bucketContext;
   populateBucketContext(bucketContext, container_name);

   S3ResponseHandler responseHandler =
   { 
      &responsePropertiesCallback,
      &responseCompleteCallback
   };

   do {
      S3_head_object(&bucketContext,       // const S3BucketContext *bucketContext
                     object_name.c_str(),  // const char *key
		     NULL,                 // S3RequestContext *requestContext
		     &responseHandler,     // const S3ResponseHandler *handler
		     &properties);         // void* callbackData
   } while (S3_status_is_retryable(statusG) && should_retry());
   
   if (statusG == S3StatusOK) {
      success = true;
   }
   */

   return success;
}

//*****************************************************************************

bool S3StorageSystem::put_object(const string& container_name,
                                 const string& object_name,
                                 const vector<unsigned char>& file_contents,
                                 const PropertySet* headers) {

   bool object_added = false;

   S3BucketContext bucketContext;
   populateBucketContext(bucketContext, container_name);

   uint64_t contentLength = file_contents.size(); 
   
   int metaPropertiesCount = 0;
   S3NameValue metaProperties[S3_MAX_METADATA_COUNT]; 
   int64_t expires = -1;
   S3CannedAcl cannedAcl = S3CannedAclPrivate;
   const char *contentDispositionFilename = 0;
   const char *contentEncoding = 0;
   const char *cacheControl = 0;
   const char *contentType = 0;
   const char *md5 = 0;
   int noStatus = 0;

   put_object_callback_data data;

   data.infile = 0;
   data.gb = 0;
   data.noStatus = noStatus;
   data.contentLength = contentLength;
   data.originalContentLength = contentLength;

   S3PutProperties putProperties =
   {
      contentType,
      md5,
      cacheControl,
      contentDispositionFilename,
      contentEncoding,
      expires,
      cannedAcl,
      metaPropertiesCount,
      metaProperties
   };

   S3PutObjectHandler putObjectHandler =
   {
      { &responsePropertiesCallback, &responseCompleteCallback },
      &putObjectDataCallback
   };

   do {
      S3_put_object(&bucketContext,       // const S3BucketContext *bucketContext
                    object_name.c_str(),  // const char *key
                    contentLength,        // uint64_t contentLength
                    &putProperties,       // const S3PutProperties*
                    NULL,                 // S3RequestContext*
                    &putObjectHandler,    // const S3PutObjectHandler*
                    &data);               // void* callbackData
   } while (S3_status_is_retryable(statusG) && should_retry());

   return object_added;
}

//*****************************************************************************

bool S3StorageSystem::delete_object(const string& container_name,
                                    const string& object_name) {
   if (debug_mode) {
      printf("delete_object: container=%s, object=%s\n",
             container_name.c_str(), object_name.c_str());
   }

   bool object_deleted = false;

   S3BucketContext bucketContext;
   populateBucketContext(bucketContext, container_name);
   
   S3ResponseHandler responseHandler =
   { 
      0,
      &responseCompleteCallback
   };

   do {
      S3_delete_object(&bucketContext,       // const S3BucketContext *bucketContext
                       object_name.c_str(),  // const char *key
                       NULL,                 // S3RequestContext* requestContext
                       &responseHandler,     // const S3ResponseHandler* handler
                       NULL);                // void* callbackData
   } while (S3_status_is_retryable(statusG) && should_retry());
   
   return object_deleted;
}

//*****************************************************************************

int64_t S3StorageSystem::get_object(const string& container_name,
                                    const string& object_name,
                                    const string& local_file_path) {
   if (debug_mode) {
      printf("get_object: container=%s, object=%s, local_file_path=%s\n",
             container_name.c_str(), object_name.c_str(),
             local_file_path.c_str());
   }
   
   if (local_file_path.length() == 0) {
      printf("error: local file path is empty\n");
      return 0;
   }
   
   FILE* outputFile = fopen(local_file_path.c_str(), "w");
   if (outputFile == NULL) {
      printf("error: unable to open output file '%s'\n", local_file_path.c_str());
      return 0;
   }

   S3BucketContext bucketContext;
   populateBucketContext(bucketContext, container_name);

   int64_t ifModifiedSince = -1;
   int64_t ifNotModifiedSince = -1;
   const char *ifMatch = 0;
   const char *ifNotMatch = 0;

   S3GetConditions getConditions =
   {
      ifModifiedSince,
      ifNotModifiedSince,
      ifMatch,
      ifNotMatch
   };

   uint64_t startByte = 0;
   uint64_t byteCount = 0;
   
   get_object_callback_data data;
   memset(&data, 0, sizeof(get_object_callback_data));
   
   data.outfile = outputFile;

   S3GetObjectHandler getObjectHandler =
   {
      { &responsePropertiesCallback, &responseCompleteCallback },
      &getObjectDataCallback
   };

   do {
      S3_get_object(&bucketContext,       // bucket context
                    object_name.c_str(),  // key
                    &getConditions,       // getConditions
                    startByte,            // startByte
                    byteCount,            // byteCount
                    NULL,                 // S3RequestContext* requestContext
                    &getObjectHandler,    // const S3GetObjectHandler *handler
                    &data);               // callback data
   } while (S3_status_is_retryable(statusG) && should_retry());
   
   fclose(outputFile);

   if (statusG == S3StatusOK) {
      return data.bytes_retrieved;
   } else {
      //TODO: delete file
      return 0;
   }
}

//*****************************************************************************

void S3StorageSystem::populateBucketContext(S3BucketContext& context,
                                            const string& container_name) {
   context.hostName = s3_host.c_str();
   context.bucketName = container_name.c_str();
   context.protocol = s3_protocol;
   context.uriStyle = s3_uri_style;
   context.accessKeyId = aws_access_key.c_str();
   context.secretAccessKey = aws_secret_key.c_str();
}

//*****************************************************************************

