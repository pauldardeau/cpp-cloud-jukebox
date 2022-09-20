#ifndef STORAGE_SYSTEM_H
#define STORAGE_SYSTEM_H

#include <string>
#include <set>

#include "file_metadata.h"
#include "utils.h"
#include "OSUtils.h"
#include "data_types.h"
#include "property_set.h"


class StorageSystem {
public:
   bool debug_mode;
   bool authenticated;
   bool compress_files;
   bool encrypt_files;
   std::set<std::string> list_containers;
   std::string container_prefix;
   std::string metadata_prefix;
   std::string storage_system_type;

   StorageSystem(const std::string& system_type, bool debug = false) :
      debug_mode(debug),
      authenticated(false),
      compress_files(false),
      encrypt_files(false),
      storage_system_type(system_type) {
   }

   virtual ~StorageSystem() {}

   std::string un_prefixed_container(const std::string& container_name) {
      if (container_prefix.length() > 0 && container_name.length() > 0) {
         // does the container name begin with the prefix?
         if (container_name.rfind(container_prefix, 0) == 0) {
            int unprefixed_len = container_name.length() - container_prefix.length();
            return container_name.substr(container_prefix.length(),
                                         unprefixed_len);
         }
      }
      return container_name;
   }

   std::string prefixed_container(const std::string& container_name) {
      return container_prefix + container_name;
   }

   bool has_container(const std::string& container_name) {
      auto it = list_containers.find(container_name);
      return it != list_containers.end();
   }

   void add_container(const std::string& container_name) {
      list_containers.emplace(container_name);
   }

   void remove_container(const std::string& container_name) {
      auto it = list_containers.find(container_name);
      const auto it_end = list_containers.end();
      if (it != it_end) {
         list_containers.erase(it);
      }
   }

   int retrieve_file(const FileMetadata& fm, const std::string& local_directory) {
      if (local_directory.length() > 0) {
         std::string file_path = chaudiere::OSUtils::pathJoin(local_directory, fm.file_uid);
         // print("retrieving container=%s" % fm.container_name);
         // print("retrieving object=%s" % fm.object_name);
         return get_object(fm.container_name, fm.object_name, file_path);
      }
      return 0;
   }

   bool store_file(const FileMetadata& fm, std::vector<unsigned char>& file_contents) {
      if (file_contents.size() > 0) {
         PropertySet dict_props;
         fm.to_dictionary(dict_props, metadata_prefix);
         return put_object(fm.container_name,
                           fm.object_name,
                           file_contents,
                           &dict_props);
      }
      return false;
   }

   bool add_file_from_path(const std::string& container_name,
                           const std::string& object_name,
                           const std::string& file_path) {
      if (container_name.length() > 0 &&
          object_name.length() > 0 &&
          file_path.length() > 0) {

         std::vector<unsigned char> file_contents;
         bool success = Utils::file_read_all_bytes(file_path, file_contents);
         if (success && file_contents.size() > 0) {
            return put_object(container_name, object_name, file_contents, NULL);
         } else {
            printf("error: unable to read file %s\n", file_path.c_str());
            return false;
         }
      } else {
         return false;
      }
   }

   virtual std::vector<std::string> list_account_containers() = 0;

   virtual bool create_container(const std::string& container_name) = 0;

   virtual bool delete_container(const std::string& container_name) = 0;

   virtual std::vector<std::string> list_container_contents(const std::string& container_name) = 0;

   virtual bool get_object_metadata(const std::string& container_name,
                                    const std::string& object_name,
                                    PropertySet& dict_props) = 0;

   virtual bool put_object(const std::string& container_name,
                           const std::string& object_name,
                           const std::vector<unsigned char>& object_bytes,
                           const PropertySet* headers=NULL) = 0;

   virtual bool delete_object(const std::string& container_name,
                              const std::string& object_name) = 0;

   virtual int get_object(const std::string& container_name,
                          const std::string& object_name,
                          const std::string& local_file_path) = 0;

   virtual bool enter() = 0;
   virtual void exit() = 0;
};

#endif

