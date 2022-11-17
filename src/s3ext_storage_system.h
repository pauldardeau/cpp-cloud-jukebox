#ifndef S3EXT_STORAGE_SYSTEM_H
#define S3EXT_STORAGE_SYSTEM_H

#include <string>
#include <vector>

#include "storage_system.h"
#include "property_set.h"
#include "KeyValuePairs.h"

// Helpful links:
// https://docs.ceph.com/en/latest/radosgw/s3/csharp/


class S3ExtStorageSystem : public StorageSystem {
private:
   bool debug_mode;
   std::string aws_access_key;
   std::string aws_secret_key;
   std::string s3_host;
   bool connected;

   S3ExtStorageSystem(const S3ExtStorageSystem&);
   S3ExtStorageSystem& operator=(const S3ExtStorageSystem&);

public:
   S3ExtStorageSystem(const std::string& aws_access_key,
                      const std::string& aws_secret_key,
                      const std::string& protocol,
                      const std::string& host,
                      const std::string& container_prefix,
                      bool debug_mode=true);
   virtual ~S3ExtStorageSystem();

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

   bool put_object(const std::string& container_name,
                   const std::string& object_name,
                   const std::string& file_path,
                   const PropertySet* headers=NULL);

   bool delete_object(const std::string& container_name,
                      const std::string& object_name);

   int64_t get_object(const std::string& container_name,
                      const std::string& object_name,
                      const std::string& local_file_path);

protected:
   void populate_common_variables(chaudiere::KeyValuePairs& kvp);
   void populate_bucket(chaudiere::KeyValuePairs& kvp,
                        const std::string& bucket_name);
   void populate_object(chaudiere::KeyValuePairs& kvp,
                        const std::string& object_name);
   bool run_program(const std::string& program_path,
                    std::vector<std::string>& list_output_lines);
   bool run_program(const std::string& program_path,
                    std::string& std_out);
   bool run_program(const std::string& program_path);
   bool prepare_run_script(const std::string& script_template,
                           const chaudiere::KeyValuePairs& kvp);
   std::string run_script_name_for_template(const std::string& script_template);

};

#endif

