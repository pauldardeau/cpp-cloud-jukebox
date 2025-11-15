#ifndef STORAGE_SYSTEM_H
#define STORAGE_SYSTEM_H

#include <string>
#include <vector>

#include "file_metadata.h"
#include "utils.h"
#include "OSUtils.h"
#include "data_types.h"
#include "property_set.h"


class StorageSystem {
private:
   bool m_debug_mode;
   bool m_authenticated;
   bool m_compress_files;
   bool m_encrypt_files;
   std::vector<std::string> m_list_containers;
   std::string m_container_prefix;
   std::string m_metadata_prefix;
   std::string m_storage_system_type;

public:
   StorageSystem(const std::string& system_type, bool debug=false) :
      m_debug_mode(debug),
      m_authenticated(false),
      m_compress_files(false),
      m_encrypt_files(false),
      m_storage_system_type(system_type) {
   }

   virtual ~StorageSystem() {}

   void set_authenticated(bool b) {
      m_authenticated = b;
   }

   void set_list_containers(const std::vector<std::string>& list_containers) {
      m_list_containers = list_containers;
   }

   bool debug_mode() const {
      return m_debug_mode;
   }

   std::string un_prefixed_container(const std::string& container_name) {
      if (!m_container_prefix.empty() && !container_name.empty()) {
         // does the container name begin with the prefix?
         if (container_name.rfind(m_container_prefix, 0) == 0) {
            int unprefixed_len = container_name.length() - m_container_prefix.length();
            return container_name.substr(m_container_prefix.length(),
                                         unprefixed_len);
         }
      }
      return container_name;
   }

   std::string prefixed_container(const std::string& container_name) const {
      return m_container_prefix + container_name;
   }

   bool has_container(const std::string& container_name) const {
      bool container_found = false;
      for (const auto& cName : m_list_containers) {
         if (cName == container_name) {
            container_found = true;
            break;
         }
      }
      return container_found;
   }

   void add_container(const std::string& container_name) {
      m_list_containers.push_back(container_name);
   }

   void remove_container(const std::string& container_name) {
      auto it = m_list_containers.begin();
      const auto it_end = m_list_containers.end();
      for (; it != it_end; it++) {
         if (*it == container_name) {
            m_list_containers.erase(it);
            break;
         }
      }
   }

   int retrieve_file(const FileMetadata& fm, const std::string& local_directory) {
      if (!local_directory.empty()) {
         std::string file_path = chaudiere::OSUtils::pathJoin(local_directory,
                                                              fm.get_file_uid());
         // print("retrieving container=%s" % fm.container_name);
         // print("retrieving object=%s" % fm.object_name);
         return get_object(fm.get_container_name(), fm.get_object_name(), file_path);
      }
      return 0;
   }

   bool store_file(const FileMetadata& fm, std::vector<unsigned char>& file_contents) {
      if (!file_contents.empty()) {
         PropertySet dict_props;
         fm.to_dictionary(dict_props, m_metadata_prefix);
         return put_object(fm.get_container_name(),
                           fm.get_object_name(),
                           file_contents,
                           &dict_props);
      }
      return false;
   }

   bool add_file_from_path(const std::string& container_name,
                           const std::string& object_name,
                           const std::string& file_path) {
      if (!container_name.empty() &&
          !object_name.empty() &&
          !file_path.empty()) {

         std::vector<unsigned char> file_contents;
         bool success = Utils::file_read_all_bytes(file_path, file_contents);
         if (success && !file_contents.empty()) {
            return put_object(container_name, object_name, file_contents, nullptr);
         } else {
            printf("error: unable to read file %s\n", file_path.c_str());
            return false;
         }
      } else {
         return false;
      }
   }

   virtual bool put_object(const std::string& container_name,
                           const std::string& object_name,
                           const std::string& file_path,
                           const PropertySet* headers=nullptr) {
      bool file_read = false;
      bool put_success = false;
      std::vector<unsigned char> file_contents;

      if (Utils::file_read_all_bytes(file_path, file_contents)) {
         file_read = true;
      } else {
         printf("error: unable to read file %s\n", file_path.c_str());
      }

      if (file_read) {
         put_success = put_object(container_name,
                                  object_name,
                                  file_contents,
                                  headers);
      }
      return put_success;
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
                           const PropertySet* headers=nullptr) = 0;

   virtual bool put_object_from_file(const std::string& container_name,
                                     const std::string& object_name,
                                     const std::string& object_file_path,
                                     const PropertySet* headers=nullptr) = 0;

   virtual bool delete_object(const std::string& container_name,
                              const std::string& object_name) = 0;

   virtual int64_t get_object(const std::string& container_name,
                              const std::string& object_name,
                              const std::string& local_file_path) = 0;

   virtual bool enter() = 0;
   virtual void exit() = 0;
};

#endif

