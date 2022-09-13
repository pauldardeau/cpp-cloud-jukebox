#include <stdio.h>
#include <strstream>
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
   aws_secret_key(secret_key),
   client(NULL) {

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
   if (client != NULL) {
      exit();
   }
}

bool S3StorageSystem::enter() {
   if (debug_mode) {
      printf("S3StorageSystem.enter\n");
      printf("attempting to connect to S3\n");
   }

   minio::s3::BaseUrl base_url("https://s3.us-central-1.wasabisys.com");

   minio::creds::StaticProvider provider(
      aws_access_key, aws_secret_key);

   client = new minio::s3::Client(base_url, &provider);

   authenticated = true;
   //list_containers = list_account_containers();
   return true;
}

void S3StorageSystem::exit() {
   if (debug_mode) {
      printf("S3StorageSystem.exit\n");
   }

   if (client != NULL) {
      if (debug_mode) {
         printf("closing S3 connection object\n");
      }

      authenticated = false;
      client = NULL;
   }
}

vector<string> S3StorageSystem::list_account_containers() {
   if (debug_mode) {
      printf("list_account_containers\n");
   }

   if (client != NULL) {
      minio::s3::ListBucketsResponse resp = client->ListBuckets();
      if (resp) {
         vector<string> list_containers;
         for (auto& bucket : resp.buckets) {
            list_containers.push_back(bucket.name);
         }
      } else {
         printf("error: unable to list containers - %s\n", resp.Error().String().c_str());
      }
   }

   return vector<string>();
}

bool S3StorageSystem::create_container(const string& container_name) {
   if (debug_mode) {
      printf("create_container: %s\n", container_name.c_str());
   }

   bool container_created = false;

   if (client != NULL) {
      minio::s3::MakeBucketArgs args;
      args.bucket = container_name;
      minio::s3::MakeBucketResponse resp = client->MakeBucket(args);
      if (resp) {
         container_created = true;
      } else {
         printf("error: unable to create container - %s\n", resp.Error().String().c_str());
      }
   }

   return container_created;
}

bool S3StorageSystem::delete_container(const string& container_name) {
   if (debug_mode) {
      printf("delete_container: %s\n", container_name.c_str());
   }

   bool container_deleted = false;
   if (client != NULL) {
      minio::s3::RemoveBucketArgs args;
      args.bucket = container_name;

      minio::s3::RemoveBucketResponse resp = client->RemoveBucket(args);
      if (resp) {
         container_deleted = true;
      } else {
         printf("error: unable to delete container - %s\n", resp.Error().String().c_str());
      }
   }

   return container_deleted;
}

vector<string> S3StorageSystem::list_container_contents(const string& container_name) {
   if (debug_mode) {
      printf("list_container_contents: %s\n", container_name.c_str());
   }

   vector<string> list_objects;

   if (client != NULL) {
      minio::s3::ListObjectsArgs args;
      args.bucket = container_name;
      minio::s3::ListObjectsResult result = client->ListObjects(args);
      for (; result; result++) {
         minio::s3::Item item = *result;
	 if (item) {
            list_objects.push_back(item.name);
	 }
      }
   }

   return list_objects;
}

bool S3StorageSystem::get_object_metadata(const string& container_name,
                                          const string& object_name,
                                          PropertySet& properties) {
   if (debug_mode) {
      printf("get_object_metadata: container=%s, object=%s\n",
             container_name.c_str(), object_name.c_str());
   }

   if (client != NULL) {
      minio::s3::StatObjectArgs args;
      args.bucket = "my-bucket";
      args.object = "my-object";

      minio::s3::StatObjectResponse resp = client->StatObject(args);
      if (resp) {
         //TODO: populate metadata
	 /*
         std::cout << "Version ID: " << resp.version_id << std::endl;
         std::cout << "ETag: " << resp.etag << std::endl;
         std::cout << "Size: " << resp.size << std::endl;
         std::cout << "Last Modified: " << resp.last_modified << std::endl;
         std::cout << "Retention Mode: ";
         if (minio::s3::IsRetentionModeValid(resp.retention_mode)) {
            std::cout << minio::s3::RetentionModeToString(resp.retention_mode)
                << std::endl;
         } else {
            std::cout << "-" << std::endl;
         }
         std::cout << "Retention Retain Until Date: ";
         if (resp.retention_retain_until_date) {
            std::cout << resp.retention_retain_until_date.ToHttpHeaderValue()
                << std::endl;
         } else {
            std::cout << "-" << std::endl;
         }
         std::cout << "Legal Hold: ";
         if (minio::s3::IsLegalHoldValid(resp.legal_hold)) {
            std::cout << minio::s3::LegalHoldToString(resp.legal_hold) << std::endl;
         } else {
            std::cout << "-" << std::endl;
         }
         std::cout << "Delete Marker: "
              << minio::utils::BoolToString(resp.delete_marker) << std::endl;
         std::cout << "User Metadata: " << std::endl;
         std::list<std::string> keys = resp.user_metadata.Keys();
         for (auto& key : keys) {
            std::cout << "  " << key << ": " << resp.user_metadata.GetFront(key)
                << std::endl;
         }
	 */
         return true;
      } else {
         printf("error: unable to retrieve metadata - %s\n", resp.Error().String().c_str());
      }
   }

   return false;
}

bool S3StorageSystem::put_object(const string& container_name,
                                 const string& object_name,
                                 const vector<unsigned char>& file_contents,
                                 const PropertySet* headers) {

   bool object_added = false;

   if (client != NULL) {
      //PutObjectArgs(std::istream &stream, long object_size, long part_size)
      std::istrstream stream(reinterpret_cast<const char*>(file_contents.data()), file_contents.size());
      minio::s3::PutObjectArgs args(stream, file_contents.size(), 0);
      args.bucket = container_name;
      args.object = object_name;

      minio::s3::PutObjectResponse resp = client->PutObject(args);

      if (resp) {
         object_added = true;
      } else {
         printf("error: unable to put object - %s\n", resp.Error().String().c_str());
      }
   }

   return object_added;
}

bool S3StorageSystem::delete_object(const string& container_name,
                                    const string& object_name) {
   if (debug_mode) {
      printf("delete_object: container=%s, object=%s\n",
             container_name.c_str(), object_name.c_str());
   }

   bool object_deleted = false;

   if (client != NULL) {
      minio::s3::RemoveObjectArgs args;
      args.bucket = container_name;
      args.object = object_name;

      minio::s3::RemoveObjectResponse resp = client->RemoveObject(args);
      if (resp) {
         object_deleted = true;
      } else {
         printf("error: unable to delete object - %s\n", resp.Error().String().c_str());
      }
   }

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

   if (client != NULL) {
      minio::s3::GetObjectArgs args;
      args.bucket = container_name;
      args.object = object_name;
      args.datafunc = [](minio::http::DataFunctionArgs args) -> bool {
         std::cout << args.datachunk;
         return true;
      };

      minio::s3::GetObjectResponse resp = client->GetObject(args);
      if (resp) {
      } else {
         printf("error: unable to get object - %s\n", resp.Error().String().c_str());
      }
   }

   return bytes_retrieved;
}

