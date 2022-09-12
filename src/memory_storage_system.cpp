#include "memory_storage_system.h"
#include "utils.h"

using namespace std;

MemoryStorageSystem::MemoryStorageSystem(bool debug_mode) :
   StorageSystem("Memory", debug_mode) {
}

bool MemoryStorageSystem::enter() {
   return true;
}

void MemoryStorageSystem::exit() {
}

vector<string> MemoryStorageSystem::list_account_containers() {
   vector<string> ret_vec(list_container_names);
   return ret_vec;
}

bool MemoryStorageSystem::create_container(const string& container_name) {
   bool container_created = false;
   if (!has_container(container_name)) {
      container_objects[container_name] = map<string, vector<unsigned char> >();
      container_headers[container_name] = map<string, map<string, PropertyValue*> >();
      list_container_names.push_back(container_name);
      container_created = true;
   }
   return container_created;
}

bool MemoryStorageSystem::delete_container(const string& container_name) {
   bool container_deleted = false;
   if (has_container(container_name)) {
      auto it_objects = container_objects.find(container_name);
      if (it_objects != container_objects.end()) {
         container_objects.erase(it_objects);
      }
      auto it_headers = container_headers.find(container_name);
      if (it_headers != container_headers.end()) {
         container_headers.erase(it_headers);
      }
      auto it_list = list_container_names.begin();
      const auto it_list_end = list_container_names.end();
      for (; it_list != it_list_end; it_list++) {
         list_container_names.erase(it_list);
         break;
      }
      container_deleted = true;
   }
   return container_deleted;
}

vector<string> MemoryStorageSystem::list_container_contents(const string& container_name) {
   vector<string> list_contents;
   auto it = container_objects.find(container_name);
   const auto it_end = container_objects.end();
   if (it != it_end) {
      const map<string, vector<unsigned char> >& object_container = it->second;
      auto it_objects = object_container.begin();
      const auto it_objects_end = object_container.end();
      for (; it_objects != it_objects_end; it_objects++) {
         list_contents.push_back(it_objects->first);
      }
   }
   return list_contents;
}

bool MemoryStorageSystem::get_object_metadata(const std::string& container_name,
                                              const std::string& object_name,
                                              PropertySet& dict_props) {
   if (container_name.length() > 0 && object_name.length() > 0) {
      auto it = container_headers.find(container_name);
      const auto it_end = container_headers.end();
      if (it != it_end) {
         auto container_obj_headers = it->second;
         auto it_obj_headers = container_obj_headers.find(object_name);
         if (it_obj_headers != container_obj_headers.end()) {
            const map<string, PropertyValue*>& header_container = it_obj_headers->second;
            auto it_props = header_container.begin();
            const auto it_props_end = header_container.end();
            for (; it_props != it_props_end; it_props++) {
               const string& prop_name = it_props->first;
               const PropertyValue* pv = it_props->second;
               dict_props.add(prop_name, pv->clone());
            }
            return true;
         }
      }
   }
   return false;
}

bool MemoryStorageSystem::put_object(const string& container_name,
                                     const string& object_name,
                                     const vector<unsigned char>& file_contents,
                                     const PropertySet* headers) {
   bool object_added = false;
   if (container_name.length() > 0 && object_name.length() > 0 && file_contents .size() > 0) {
      map<string, vector<unsigned char> > object_container =
         container_objects[container_name];
      object_container[object_name] = file_contents;
      //if (headers != null) {
      //   map<string, PropertyValue*> header_container =
      //      container_headers[container_name];
      //   header_container[object_name] = headers.Value;
      //}
      object_added = true;
   }
   return object_added;
}

bool MemoryStorageSystem::delete_object(const string& container_name,
                                        const string& object_name) {
   bool object_deleted = false;
   if (container_name.length() > 0 && object_name.length() > 0) {
      if (has_container(container_name)) {
         auto it_container = container_objects.find(container_name);
         if (it_container != container_objects.end()) {
            auto object_container = it_container->second;
            auto it_objects = object_container.find(object_name);
            if (it_objects != object_container.end()) {
               object_container.erase(it_objects);
               object_deleted = true;
            }
         }
         map<string, map<string, PropertyValue*> >& header_container =
            container_headers[container_name];
         auto it_header = header_container.find(object_name);
         const auto it_header_end = header_container.end();
         if (it_header != it_header_end) {
            header_container.erase(it_header);
            object_deleted = true;
         }
      }
   }
   return object_deleted;
}

int MemoryStorageSystem::get_object(const string& container_name,
                                    const string& object_name,
                                    const string& local_file_path) {
   int bytes_retrieved = 0;
   if (container_name.length() > 0 &&
       object_name.length() > 0 &&
       local_file_path.length() > 0) {

      auto it_container = container_objects.find(container_name);
      if (it_container != container_objects.end()) {
         auto object_container = it_container->second;
         auto it_object = object_container.find(object_name);
         if (it_object != object_container.end()) {
            const vector<unsigned char>& object_contents = it_object->second;
            Utils::file_write_all_bytes(local_file_path, object_contents);
            bytes_retrieved = object_contents.size();
         }
      }
   }
   return bytes_retrieved;
}

