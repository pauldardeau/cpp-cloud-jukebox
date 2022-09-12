#ifndef MEMORY_STORAGE_SYSTEM_H
#define MEMORY_STORAGE_SYSTEM_H

#include <string>
#include <vector>
#include <map>

#include "storage_system.h"
#include "data_types.h"

typedef std::map<std::string, std::vector<unsigned char> > dict_objects;
typedef std::map<std::string, dict_objects> dict_cont_objects;

typedef std::map<std::string, PropertyValue*> dict_properties;
typedef std::map<std::string, dict_properties> dict_obj_props;
typedef std::map<std::string, dict_obj_props> dict_cont_obj_props;

class MemoryStorageSystem : public StorageSystem {
private:
   std::vector<std::string> list_container_names;
   dict_cont_objects container_objects;
   dict_cont_obj_props container_headers;

public:
   MemoryStorageSystem(bool debug_mode = false);

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

   bool delete_object(const std::string& container_name,
                      const std::string& object_name);

   int get_object(const std::string& container_name,
                  const std::string& object_name,
		  const std::string& local_file_path);
};

#endif

