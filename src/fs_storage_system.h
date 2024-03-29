#ifndef FS_STORAGE_SYSTEM_H
#define FS_STORAGE_SYSTEM_H

#include <string>
#include <vector>
#include <map>

#include "storage_system.h"
#include "data_types.h"


class FSStorageSystem : public StorageSystem {
private:
   std::string root_dir;

   FSStorageSystem(const FSStorageSystem&);
   FSStorageSystem& operator=(const FSStorageSystem&);

public:
   FSStorageSystem(const std::string& the_root_dir, bool debug_mode = false);
   ~FSStorageSystem();

   bool enter();
   void exit();

   std::vector<std::string> list_account_containers();

   bool create_container(const std::string& container_name);

   bool delete_container(const std::string& container_name);

   std::vector<std::string> list_container_contents(const std::string& container_name);

   bool get_object_metadata(const std::string& container_name,
                            const std::string& object_name,
                            PropertySet& dict_props);

   bool put_object(const std::string& container_name,
                   const std::string& object_name,
                   const std::vector<unsigned char>& file_contents,
                   const PropertySet* headers=NULL);

   bool put_object_from_file(const std::string& container_name,
                             const std::string& object_name,
                             const std::string& object_file_path,
                             const PropertySet* headers=NULL);

   bool delete_object(const std::string& container_name,
                      const std::string& object_name);

   int64_t get_object(const std::string& container_name,
                      const std::string& object_name,
                      const std::string& local_file_path);
};

#endif

