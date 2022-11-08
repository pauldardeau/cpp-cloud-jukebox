#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <libs3.h>

#include "s3_storage_system.h"
#include "OSUtils.h"

#ifndef SLEEP_UNITS_PER_SECOND
#define SLEEP_UNITS_PER_SECOND 1
#endif

using namespace std;

//*****************************************************************************
//TODO:
// - move hostname to creds file
//*****************************************************************************

// Helpful links:
// https://docs.ceph.com/en/latest/radosgw/s3/csharp/
// https://docs.ceph.com/en/latest/radosgw/s3/cpp/

static int retriesG = 3;
static S3Status statusG = S3StatusOK;
static int showResponsePropertiesG = 0;
static char errorDetailsG[4096] = { 0 };

struct _put_object_callback_data
{
   FILE* infile;  // put_object from file
   const vector<unsigned char>* contentBuffer;  // put object from memory
   uint64_t bufferOffset;
   uint64_t contentLength;
};

typedef struct _put_object_callback_data put_object_callback_data;

struct _list_bucket_callback_data
{
   int isTruncated;
   char nextMarker[1024];
   int keyCount;
   vector<string>* list_objects;
};

typedef struct _list_bucket_callback_data list_bucket_callback_data;

struct _get_object_callback_data
{
   FILE* outfile;
   int64_t bytes_retrieved;
};

typedef struct _get_object_callback_data get_object_callback_data;

//*****************************************************************************

static bool should_retry()
{
   if (retriesG--) {
      // Sleep before next retry; start out with a 1 second sleep
      static int retrySleepInterval = 1 * SLEEP_UNITS_PER_SECOND;
      sleep(retrySleepInterval);
      // Next sleep 1 second longer
      retrySleepInterval++;
      return true;
   }

   return false;
}

//*****************************************************************************

static S3Status listServiceCallback(const char *ownerId, 
                                    const char *ownerDisplayName,
                                    const char *bucketName,
                                    int64_t creationDate,
                                    void *callbackData)
{
   vector<string>* list_containers = (vector<string>*) callbackData;
   list_containers->push_back(string(bucketName));
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
   } else {
      data->nextMarker[0] = 0;
   }
    
   for (int i = 0; i < contentsCount; i++) {
      const S3ListBucketContent *content = &(contents[i]);
      data->list_objects->push_back(string(content->key));
   }

   data->keyCount += contentsCount;

   return S3StatusOK;
}

//*****************************************************************************

static S3Status responsePropertiesCallback(const S3ResponseProperties *properties,
                                           void *callbackData)
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
      memset(timebuf, 0, sizeof(timebuf));
      time_t t = (time_t) properties->lastModified;
      // gmtime is not thread-safe but we don't care here.
      strftime(timebuf, sizeof(timebuf), "%Y-%m-%dT%H:%M:%SZ", gmtime(&t));
      props->add("Last-Modified", new StrPropertyValue(timebuf));
   }
   
   for (int i = 0; i < properties->metaDataCount; i++) {
      char propName[256];
      memset(propName, 0, sizeof(propName));
      snprintf(propName, 256, "x-amz-meta-%s", properties->metaData[i].name);
      props->add(propName,
                 new StrPropertyValue(properties->metaData[i].value));
   }

   return S3StatusOK;
}

//*****************************************************************************

static int putObjectDataCallback(int bufferSize,
                                 char *buffer,
                                 void *callbackData)
{
   put_object_callback_data *data = 
       (put_object_callback_data *) callbackData;
    
   int ret = 0;

   if (data->contentLength) {
      int toRead = ((data->contentLength > (unsigned) bufferSize) ?
                    (unsigned) bufferSize : data->contentLength);
      if (data->infile) {
         ret = fread(buffer, 1, toRead, data->infile);
      } else if (data->contentBuffer) {
         //TODO: fill buffer from contentBuffer (use std::copy)
      }
   }

   data->contentLength -= ret;

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
      for (int i = 0; i < error->extraDetailsCount; i++) {
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
   debug_mode(debug),
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
   }

   S3Status status = S3_initialize("s3", S3_INIT_ALL, s3_host.c_str());
   if (status == S3StatusOK) {
      authenticated = true;
      connected = true;
      list_containers = list_account_containers();
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

   vector<string> list_containers;

   showResponsePropertiesG = false;
   S3ListServiceHandler listServiceHandler =
   {
      { &responsePropertiesCallback, &responseCompleteCallback },
      &listServiceCallback
   };

   do {
      S3_list_service(s3_protocol,            // S3Protocol protocol
                      aws_access_key.c_str(), // const char *accessKeyId
                      aws_secret_key.c_str(), // const char *secretAccessKey
                      s3_host.c_str(),        // const char *hostName
                      NULL,                   // S3RequestContext *requestContext
                      &listServiceHandler,    // const S3ListServiceHandler *handler
                      &list_containers);      // void *callbackData
   } while (S3_status_is_retryable(statusG) && should_retry());

   return list_containers;
}

//*****************************************************************************

bool S3StorageSystem::create_container(const string& container_name) {
   if (debug_mode) {
      printf("create_container: %s\n", container_name.c_str());
   }

   bool container_created = false;

   const char *locationConstraint = 0;
   S3CannedAcl cannedAcl = S3CannedAclPrivate;

   showResponsePropertiesG = false;
   S3ResponseHandler responseHandler =
   {
      &responsePropertiesCallback, &responseCompleteCallback
   };

   do {
      S3_create_bucket(s3_protocol,            // S3Protocol protocol
                       aws_access_key.c_str(), // const char *accessKeyId
                       aws_secret_key.c_str(), // const char *secretAccessKey
                       s3_host.c_str(),        // const char *hostName
                       container_name.c_str(), // const char *bucketName
                       cannedAcl,              // S3CannedAcl cannedAcl
                       locationConstraint,     // const char *locationConstraint
                       NULL,                   // S3RequestContext *requestContext
                       &responseHandler,       // const S3ResponseHandler *handler
                       NULL);                  // void *callbackData
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

   showResponsePropertiesG = false;
   S3ResponseHandler responseHandler =
   {
      &responsePropertiesCallback, &responseCompleteCallback
   };

   do {
      S3_delete_bucket(s3_protocol,            // S3Protocol protocol
                       s3_uri_style,           // S3UriStyle uriStyle
                       aws_access_key.c_str(), // const char *accessKeyId
                       aws_secret_key.c_str(), // const char *secretAccessKey
                       s3_host.c_str(),        // const char *hostName
                       container_name.c_str(), // const char *bucketName
                       NULL,                   // S3RequestContext *requestContext
                       &responseHandler,       // const S3ResponseHandler *handler
                       NULL);                  // void *callbackData
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
   
   showResponsePropertiesG = false;
   S3ListBucketHandler listBucketHandler =
   {
      { &responsePropertiesCallback, &responseCompleteCallback },
      &listBucketCallback
   };

   int maxkeys = 0;
   
   list_bucket_callback_data data;
   memset(&data, 0, sizeof(list_bucket_callback_data));

   data.keyCount = 0;
   data.list_objects = &list_objects;

   do {
      data.isTruncated = 0;
      do {
         S3_list_bucket(&bucketContext,     // const S3BucketContext *bucketContext
                        0,                  // const char *prefix
                        data.nextMarker,    // const char *marker
                        0,                  // const char *delimiter
                        0,                  // int maxkeys
                        NULL,               // S3RequestContext*
                        &listBucketHandler, // const S3ListBucketHandler *handler
                        &data);             // void *callbackData
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
   showResponsePropertiesG = true;

   S3BucketContext bucketContext;
   populateBucketContext(bucketContext, container_name);

   S3ResponseHandler responseHandler =
   { 
      &responsePropertiesCallback,
      &responseCompleteCallback
   };

   do {
      S3_head_object(&bucketContext,      // const S3BucketContext *bucketContext
                     object_name.c_str(), // const char *key
		     NULL,                // S3RequestContext *requestContext
		     &responseHandler,    // const S3ResponseHandler *handler
		     &properties);        // void* callbackData
   } while (S3_status_is_retryable(statusG) && should_retry());
   
   if (statusG == S3StatusOK) {
      success = true;
   }
   
   showResponsePropertiesG = false;

   return success;
}

//*****************************************************************************

bool S3StorageSystem::put_object(const string& container_name,
                                 const string& object_name,
                                 const vector<unsigned char>& file_contents,
                                 const PropertySet* headers) {

   if (debug_mode) {
      printf("put_object: container=%s, object=%s, length=%ld\n",
             container_name.c_str(),
             object_name.c_str(),
             file_contents.size());
   }
   
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

   put_object_callback_data data;
   memset(&data, 0, sizeof(put_object_callback_data));
   
   data.contentBuffer = &file_contents;
   data.contentLength = contentLength;

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

   showResponsePropertiesG = false;
   S3PutObjectHandler putObjectHandler =
   {
      { &responsePropertiesCallback, &responseCompleteCallback },
      &putObjectDataCallback
   };

   do {
      S3_put_object(&bucketContext,      // const S3BucketContext *bucketContext
                    object_name.c_str(), // const char *key
                    contentLength,       // uint64_t contentLength
                    &putProperties,      // const S3PutProperties*
                    NULL,                // S3RequestContext*
                    &putObjectHandler,   // const S3PutObjectHandler*
                    &data);              // void* callbackData
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
      S3_delete_object(&bucketContext,      // const S3BucketContext *bucketContext
                       object_name.c_str(), // const char *key
                       NULL,                // S3RequestContext* requestContext
                       &responseHandler,    // const S3ResponseHandler* handler
                       NULL);               // void* callbackData
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
      printf("error: unable to open output file '%s'\n",
             local_file_path.c_str());
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

   showResponsePropertiesG = false;
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
      chaudiere::OSUtils::deleteFile(local_file_path);
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

