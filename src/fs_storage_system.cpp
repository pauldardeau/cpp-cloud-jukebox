#include "fs_storage_system.h"
#include "utils.h"
#include "OSUtils.h"

using namespace std;

FSStorageSystem::FSStorageSystem(const string& the_root_dir, bool debug_mode) :
   StorageSystem("FS", debug_mode),
   root_dir(the_root_dir) {
}

bool FSStorageSystem::enter() {
   if (!chaudiere::OSUtils::directoryExists(root_dir)) {
      chaudiere::OSUtils::createDirectory(root_dir);
   }
   return chaudiere::OSUtils::directoryExists(root_dir);
}

void FSStorageSystem::exit() {
}

vector<string> FSStorageSystem::list_account_containers() {
   vector<string> ret_vec(list_container_names);
   return ret_vec;   
}

bool FSStorageSystem::create_container(const string& container_name) {
   bool container_created = false;
   if (!has_container(container_name)) {
      string container_dir = chaudiere::OSUtils::pathJoin(root_dir, container_name);
      container_created = chaudiere::OSUtils::createDirectory(container_dir);
      if (container_created) {
         list_container_names.push_back(container_name);
      }
   }
   return container_created;
}

bool FSStorageSystem::delete_container(const string& container_name) {
   bool container_deleted = false;
   if (has_container(container_name)) {
      string container_dir = chaudiere::OSUtils::pathJoin(root_dir, container_name);
      container_deleted = Utils::directory_delete_directory(container_dir);
      if (container_deleted) {
         //TODO: remove it from list
      }
   }
   return container_deleted;
}

vector<string> FSStorageSystem::list_container_contents(const string& container_name) {
   vector<string> list_contents;
   string container_dir = chaudiere::OSUtils::pathJoin(root_dir, container_name);
   if (chaudiere::OSUtils::directoryExists(container_dir)) {
      return chaudiere::OSUtils::listFilesInDirectory(container_dir);
   }
   return list_contents;
}

bool FSStorageSystem::get_object_metadata(const std::string& container_name,
                                          const std::string& object_name,
                                          PropertySet& dict_props) {
   if (container_name.length() > 0 && object_name.length() > 0) {
      string container_dir = chaudiere::OSUtils::pathJoin(root_dir, container_name);
      if (chaudiere::OSUtils::directoryExists(container_dir)) {
         string object_path = chaudiere::OSUtils::pathJoin(container_dir, object_name);
         string meta_path = object_path + ".meta";
         if (Utils::file_exists(meta_path)) {
            return dict_props.read_from_file(meta_path);
         }
      }
   }
   return false;
}

bool FSStorageSystem::put_object(const string& container_name,
                                 const string& object_name,
                                 const vector<unsigned char>& file_contents,
                                 const PropertySet* headers) {
   bool object_added = false;
   if (container_name.length() > 0 && object_name.length() > 0 && file_contents .size() > 0) {
      string container_dir = chaudiere::OSUtils::pathJoin(root_dir, container_name);
      if (chaudiere::OSUtils::directoryExists(container_dir)) {
         string object_path = chaudiere::OSUtils::pathJoin(container_dir, object_name);
         object_added = Utils::file_write_all_bytes(object_path, file_contents);
	 if (headers != NULL) {
            if (headers->count() > 0) {
               string meta_path = object_path + ".meta";
	       headers->write_to_file(meta_path);
            }
         }
      }
   }
   return object_added;
}

bool FSStorageSystem::delete_object(const string& container_name,
                                    const string& object_name) {
   bool object_deleted = false;
   if (container_name.length() > 0 && object_name.length() > 0) {
      if (has_container(container_name)) {
         string container_dir = chaudiere::OSUtils::pathJoin(root_dir, container_name);
         string object_path = chaudiere::OSUtils::pathJoin(container_dir, object_name);
         if (Utils::file_exists(object_path)) {
            object_deleted = chaudiere::OSUtils::deleteFile(object_path);
	    string meta_path = object_path + ".meta";
	    if (Utils::file_exists(meta_path)) {
               chaudiere::OSUtils::deleteFile(meta_path);
            }
         }
      }
   }
   return object_deleted;
}

int FSStorageSystem::get_object(const string& container_name,
                                const string& object_name,
                                const string& local_file_path) {
   int bytes_retrieved = 0;
   if (container_name.length() > 0 &&
       object_name.length() > 0 &&
       local_file_path.length() > 0) {

      string container_dir = chaudiere::OSUtils::pathJoin(root_dir, container_name);
      string object_path = chaudiere::OSUtils::pathJoin(container_dir, object_name);
      if (Utils::file_exists(object_path)) {
         //TODO: read object contents and write to local_file_path
         //Utils::file_write_all_bytes(local_file_path, object_contents);
         //bytes_retrieved = object_contents.size();
      }
   }
   return bytes_retrieved;
}

