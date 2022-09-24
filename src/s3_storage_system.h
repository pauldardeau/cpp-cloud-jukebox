#ifndef S3_STORAGE_SYSTEM_H
#define S3_STORAGE_SYSTEM_H

#include <string>
#include <vector>

#include "storage_system.h"
#include "property_set.h"

// minio client
#include "client.h"

// Helpful links:
// https://docs.ceph.com/en/latest/radosgw/s3/csharp/


class S3StorageSystem : public StorageSystem {
private:
   bool debug_mode;
   std::string aws_access_key;
   std::string aws_secret_key;
   minio::s3::Client* client;

   S3StorageSystem(const S3StorageSystem&);
   S3StorageSystem& operator=(const S3StorageSystem&);

public:
   S3StorageSystem(const std::string& aws_access_key,
                   const std::string& aws_secret_key,
                   const std::string& container_prefix,
                   bool debug_mode=true);
   virtual ~S3StorageSystem();

   bool enter();
   void exit();

   std::vector<std::string> list_account_containers();

   bool create_container(const std::string& container_name);
   bool delete_container(const std::string& container_name);
   std::vector<std::string> list_container_contents(const std::string& container_name);

   bool get_object_metadata(const std::string& container_name,
                            const std::string& object_name,
                            PropertySet& properties);

   bool put_object(const std::string& container_name,
                   const std::string& object_name,
                   const std::vector<unsigned char>& file_contents,
                   const PropertySet* headers=NULL);

   bool delete_object(const std::string& container_name,
                      const std::string& object_name);

   int get_object(const std::string& container_name,
                  const std::string& object_name,
                  const std::string& local_file_path);

};

#endif

