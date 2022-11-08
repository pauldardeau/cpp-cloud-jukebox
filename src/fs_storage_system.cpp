#include "fs_storage_system.h"
#include "utils.h"
#include "OSUtils.h"

using namespace std;

FSStorageSystem::FSStorageSystem(const string& the_root_dir, bool debug_mode) :
   StorageSystem("FS", debug_mode),
   root_dir(the_root_dir) {
}

FSStorageSystem::~FSStorageSystem() {
   exit();
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
   return chaudiere::OSUtils::listDirsInDirectory(root_dir);
}

bool FSStorageSystem::create_container(const string& container_name) {
   bool container_created = false;
   if (!has_container(container_name)) {
      string container_dir = chaudiere::OSUtils::pathJoin(root_dir, container_name);
      container_created = chaudiere::OSUtils::createDirectory(container_dir);
      if (container_created) {
         if (debug_mode) {
            printf("container created: '%s'\n", container_name.c_str());
         }
         add_container(container_name);
      }
   }
   return container_created;
}

bool FSStorageSystem::delete_container(const string& container_name) {
   bool container_deleted = false;
   string container_dir = chaudiere::OSUtils::pathJoin(root_dir, container_name);
   container_deleted = Utils::directory_delete_directory(container_dir);
   if (container_deleted) {
      if (debug_mode) {
         printf("container deleted: '%s'\n", container_name.c_str());
      }
      remove_container(container_name);
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
   if (container_name.length() > 0 && object_name.length() > 0 && file_contents.size() > 0) {
      string container_dir = chaudiere::OSUtils::pathJoin(root_dir, container_name);
      if (chaudiere::OSUtils::directoryExists(container_dir)) {
         string object_path = chaudiere::OSUtils::pathJoin(container_dir, object_name);
         object_added = Utils::file_write_all_bytes(object_path, file_contents);
         if (object_added) {
            if (debug_mode) {
               printf("object added: %s/%s\n", container_name.c_str(), object_name.c_str());
            }
            if (headers != NULL) {
               if (headers->count() > 0) {
                  string meta_path = object_path + ".meta";
                  headers->write_to_file(meta_path);
               }
            }
         } else {
            printf("file_write_all_bytes failed to write object contents, put failed\n");
         }
      } else {
         if (debug_mode) {
            printf("container doesn't exist, can't put object\n");
         }
      }
   } else {
      if (debug_mode) {
         if (container_name.length() == 0) {
            printf("container name is missing, can't put object\n");
         } else {
            if (object_name.length() == 0) {
               printf("object name is missing, can't put object\n");
            } else {
               if (file_contents.size() == 0) {
                  printf("object content is empty, can't put object\n");
               }
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
      string container_dir = chaudiere::OSUtils::pathJoin(root_dir, container_name);
      string object_path = chaudiere::OSUtils::pathJoin(container_dir, object_name);
      if (Utils::file_exists(object_path)) {
         object_deleted = chaudiere::OSUtils::deleteFile(object_path);
         if (object_deleted) {
            if (debug_mode) {
               printf("object deleted: %s/%s\n", container_name.c_str(), object_name.c_str());
            }
            string meta_path = object_path + ".meta";
            if (Utils::file_exists(meta_path)) {
               chaudiere::OSUtils::deleteFile(meta_path);
            }
         } else {
            if (debug_mode) {
               printf("delete of object file failed\n");
            }
         }
      } else {
         if (debug_mode) {
            printf("cannot delete object, path doesn't exist\n");
         }
      }
   } else {
      if (debug_mode) {
         printf("cannot delete object, container name or object name is missing\n");
      }
   }
   return object_deleted;
}

int64_t FSStorageSystem::get_object(const string& container_name,
                                    const string& object_name,
                                    const string& local_file_path) {
   int64_t bytes_retrieved = 0;
   if (container_name.length() > 0 &&
       object_name.length() > 0 &&
       local_file_path.length() > 0) {

      string container_dir = chaudiere::OSUtils::pathJoin(root_dir, container_name);
      string object_path = chaudiere::OSUtils::pathJoin(container_dir, object_name);
      if (Utils::file_exists(object_path)) {
         vector<unsigned char> obj_file_contents;
         if (Utils::file_read_all_bytes(object_path, obj_file_contents)) {
            if (Utils::file_write_all_bytes(local_file_path, obj_file_contents)) {
               bytes_retrieved = obj_file_contents.size();
            }
         }
      }
   }
   return bytes_retrieved;
}

