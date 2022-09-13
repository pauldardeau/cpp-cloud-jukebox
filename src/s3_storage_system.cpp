#include <stdio.h>
#include "s3_storage_system.h"

using namespace std;

// Helpful links:
// https://docs.ceph.com/en/latest/radosgw/s3/csharp/


S3StorageSystem::S3StorageSystem(const string& access_key,
                                 const string& secret_key,
                                 const string& container_prefix,
                                 bool debug) :
   StorageSystem("S3", debug),
   debug_mode(debug),
   aws_access_key(access_key),
   aws_secret_key(secret_key) {

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

S3StorageSystem::~S3StorageSystem() {
}

bool S3StorageSystem::enter() {
   if (debug_mode) {
      printf("S3StorageSystem.enter\n");
      printf("attempting to connect to S3\n");
   }

   //AmazonS3Config config = new AmazonS3Config();
   //config.ServiceURL = "https://s3.us-central-1.wasabisys.com";

   //conn = new AmazonS3Client(aws_access_key,
   //                          aws_secret_key,
   //                          config);
   //authenticated = true;
   //list_containers = list_account_containers();
   //return true;

   return false;
}

void S3StorageSystem::exit() {
   if (debug_mode) {
      printf("S3StorageSystem.exit\n");
   }

   /*
   if (conn != NULL) {
      if (debug_mode) {
         printf("closing S3 connection object\n");
      }

      authenticated = false;
      list_containers = null;
      conn = NULL;
   }
   */
}

vector<string> S3StorageSystem::list_account_containers() {
   if (debug_mode) {
      printf("list_account_containers\n");
   }
   /*
   if (conn != NULL) {
      var listResponse = do_list_account_containers(conn);
      log_api_call(listResponse.HttpStatusCode, "ListBuckets");

      if (listResponse.HttpStatusCode == HttpStatusCode.OK) {
         vector<string> list_container_names;

         foreach (S3Bucket bucket in listResponse.Buckets)
         {
            list_container_names.push_back(un_prefixed_container(bucket.BucketName));
         }

         return list_container_names;
      }
   }
   */

   return vector<string>();
}

bool S3StorageSystem::create_container(const string& container_name) {
   if (debug_mode) {
      printf("create_container: %s\n", container_name.c_str());
   }

   bool container_created = false;
   /*
   if (conn != NULL) {
      try
      {
         PutBucketRequest request = new PutBucketRequest();
         request.BucketName = container_name;
         var putBucketResponse = do_create_container(conn, request);
         log_api_call(putBucketResponse.HttpStatusCode, "PutBucket: " + container_name);
         if (putBucketResponse.HttpStatusCode == HttpStatusCode.OK) {
            add_container(container_name);
            container_created = true;
         }
      }
      catch (Exception e)
      {
         printf("PutBucket exception: " + e);
      }
   }
   */

   return container_created;
}

bool S3StorageSystem::delete_container(const string& container_name) {
   if (debug_mode) {
      printf("delete_container: %s\n", container_name.c_str());
   }

   bool container_deleted = false;
   /*
   if (conn != NULL) {
      try
      {
         string bucket_name = prefixed_container(container_name);
         DeleteBucketRequest request = new DeleteBucketRequest();
         request.BucketName = bucket_name;
         var deleteResponse = do_delete_container(conn, request);
         log_api_call(deleteResponse.HttpStatusCode, "DeleteBucket: " + bucket_name);

         if (deleteResponse.HttpStatusCode == HttpStatusCode.OK) {
            remove_container(container_name);
            container_deleted = true;
         }
      }
      catch (Exception e)
      {
         printf("DeleteBucket exception: " + e);
      }
   }
   */

   return container_deleted;
}

vector<string> S3StorageSystem::list_container_contents(const string& container_name) {
   if (debug_mode) {
      printf("list_container_contents: %s\n", container_name.c_str());
   }

   /*
   if (conn != NULL) {
      try
      {
         ListObjectsRequest request = new ListObjectsRequest();
         request.BucketName = container_name;
         var listResponse = do_list_container_contents(conn, request);
         log_api_call(listResponse.HttpStatusCode, "ListObjects: " + container_name);
         if (listResponse.HttpStatusCode == HttpStatusCode.OK) {
            vector<string> list_contents;

            foreach (S3Object obj in listResponse.S3Objects)
            {
               list_contents.push_back(obj.Key);
            }

            return list_contents;
         }
      }
      catch (Exception e)
      {
         printf("ListObjects exception: {0}", e);
      }
   }
   */

   return vector<string>();
}

bool S3StorageSystem::get_object_metadata(const string& container_name,
                                          const string& object_name,
                                          PropertySet& properties) {
   if (debug_mode) {
      printf("get_object_metadata: container=%s, object=%s\n",
             container_name.c_str(), object_name.c_str());
   }

   /*
   if (conn != NULL && container_name.length() > 0 && object_name.length() > 0) {
      try
      {
         var getObjectMetadataResponse = do_get_object_metadata(conn, container_name, object_name);
         log_api_call(getObjectMetadataResponse.HttpStatusCode, "GetObjectMetadata: " + container_name + ":" + object_name);

         if (getObjectMetadataResponse.HttpStatusCode == HttpStatusCode.OK) {
            foreach (var header_key in getObjectMetadataResponse.Headers.Keys)
            {
               string header_value = getObjectMetadataResponse.Headers[header_key];
               if (header_value != null) {
                  dict_headers[header_key] = header_value;
               }
            }
	    return true;
         }
      }
      catch (Exception e)
      {
         printf("GetObjectMetadata exception: " + e);
      }

      return false;
   }
   */

   return false;
}

bool S3StorageSystem::put_object(const string& container_name,
                                 const string& object_name,
                                 const vector<unsigned char>& file_contents,
                                 const PropertySet* headers) {

   bool object_added = false;

   /*
   if (conn != NULL && container_name.length() > 0 &&
       object_name.length() > 0 && file_contents.size() > 0) {

      try
      {
         string bucket = container_name;
         PutObjectRequest request = new PutObjectRequest();
         request.BucketName = container_name;
         request.Key = object_name;
         request.InputStream = new MemoryStream(file_contents);
         //request.Headers = ;  //TODO: (2) add headers (put_object)

         if (headers != null) {
            if (headers.ContainsKey("ContentType")) {
               string contentType = (string) headers["ContentType"];
               if (contentType != null && contentType.Length > 0) {
                  request.ContentType = contentType;
               }
            }
         }
            
         var putObjectResponse = do_put_object(conn, request);
         log_api_call(putObjectResponse.HttpStatusCode, "PutObject: " + container_name + ":" + object_name);
         if (putObjectResponse.HttpStatusCode == HttpStatusCode.OK) {
            object_added = true;
         }
      }
      catch (Exception e)
      {
         printf("PutObject exception: " + e);
      }
   }
   */

   return object_added;
}

bool S3StorageSystem::delete_object(const string& container_name,
                                    const string& object_name) {
   if (debug_mode) {
      printf("delete_object: container=%s, object=%s\n",
             container_name.c_str(), object_name.c_str());
   }

   bool object_deleted = false;

   /*
   if (conn != NULL && container_name.length() > 0 && object_name.length() > 0) {
      try
      {
         DeleteObjectRequest request = new DeleteObjectRequest();
         request.BucketName = container_name;
         request.Key = object_name;
         var deleteObjectResponse = do_delete_object(conn, request);
         log_api_call(deleteObjectResponse.HttpStatusCode, "DeleteObject: " + container_name + ":" + object_name);
         object_deleted = true;
      }
      catch (Exception e)
      {
         printf("DeleteObject exception: " + e);
      }
   }
   */

   return object_deleted;
}

int S3StorageSystem::get_object(const string& container_name,
                                const string& object_name,
				const string& local_file_path) {
   if (debug_mode) {
      printf("get_object: container=%s, object=%s, local_file_path=%s\n",
             container_name.c_str(), object_name.c_str(),
             local_file_path.c_str());
   }

   int bytes_retrieved = 0;

   /*
   if (conn != NULL && container_name.length() > 0 &&
       object_name.length() > 0 && local_file_path.length() > 0) {

      try
      {
         GetObjectRequest request = new GetObjectRequest();
         request.BucketName = container_name;
         request.Key = object_name;
         var getObjectResponse = do_get_object(conn, request);
         log_api_call(getObjectResponse.HttpStatusCode, "GetObject: " + container_name + ":" + object_name);
         if (getObjectResponse.HttpStatusCode == HttpStatusCode.OK) {
            // AWS SDK BUG: according to Amazon's docs, the following should
            // work, but it's a compilation error.
            //getObjectResponse.WriteResponseStreamToFile(local_file_path);

            // Workaround: we'll do the same thing ourselves
            using(Stream outStream = File.OpenWrite(local_file_path))
            {
               getObjectResponse.ResponseStream.CopyTo(outStream);
            }

            if (Utils::path_exists(local_file_path)) {
               bytes_retrieved = (int) Utils.get_file_size(local_file_path);
            }
         }
      }
      catch (Exception e)
      {
         printf("GetObject exception: " + e);
      }
   }
   */

   return bytes_retrieved;
}

