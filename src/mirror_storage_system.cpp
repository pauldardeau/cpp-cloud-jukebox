#include "mirror_storage_system.h"
#include "utils.h"
#include "OSUtils.h"
#include "Runnable.h"
#include "PThreadsThread.h"

using namespace std;

class CreateContainer;
class DeleteContainer;
class PutObject;
class DeleteObject;

//******************************************************************************
//******************************************************************************

UpdateOperation::UpdateOperation(const UpdateOperation& copy) :
   storage_system(NULL),
   op_name(copy.op_name) {
}

UpdateOperation& UpdateOperation::operator=(const UpdateOperation& copy) {
   if (this == &copy) {
      return *this;
   }

   op_name = copy.op_name;

   return *this;
}

//******************************************************************************
//******************************************************************************

class CreateContainer : public UpdateOperation {
private:
   string container_name;

public:
   CreateContainer(StorageSystem* ss, const string& container) :
      UpdateOperation(ss, "CreateContainer"),
      container_name(container) {
   }

   CreateContainer(const CreateContainer& copy) :
      UpdateOperation(copy),
      container_name(copy.container_name) {
   }

   CreateContainer& operator=(const CreateContainer& copy) {
      if (this == &copy) {
         return *this;
      }

      UpdateOperation::operator=(copy);

      container_name = copy.container_name;

      return *this;
   }

   virtual UpdateOperation* clone() {
      return new CreateContainer(*this);
   }

   void run() {
   }
};

//******************************************************************************
//******************************************************************************

class DeleteContainer : public UpdateOperation {
private:
   string container_name;

public:
   DeleteContainer(StorageSystem* ss, const string& container) :
      UpdateOperation(ss, "DeleteContainer"),
      container_name(container) {
   }

   DeleteContainer(const DeleteContainer& copy) :
      UpdateOperation(copy),
      container_name(copy.container_name) {
   }

   DeleteContainer& operator=(const DeleteContainer& copy) {
      if (this == &copy) {
         return *this;
      }

      UpdateOperation::operator=(copy);

      container_name = copy.container_name;

      return *this;
   }

   virtual UpdateOperation* clone() {
      return new DeleteContainer(*this);
   }

   void run() {
   }
};

//******************************************************************************
//******************************************************************************

class PutObject : public UpdateOperation {
private:
   string container_name;
   string object_name;

public:
   PutObject(StorageSystem* ss, const string& container, const string& object) :
      UpdateOperation(ss, "PutObject"),
      container_name(container),
      object_name(object) {
   }

   PutObject(const PutObject& copy) :
      UpdateOperation(copy),
      container_name(copy.container_name),
      object_name(copy.object_name) {
   }

   PutObject& operator=(const PutObject& copy) {
      if (this == &copy) {
         return *this;
      }

      UpdateOperation::operator=(copy);

      container_name = copy.container_name;
      object_name = copy.object_name;

      return *this;
   }

   virtual UpdateOperation* clone() {
      return new PutObject(*this);
   }

   void run() {
   }
};

//******************************************************************************
//******************************************************************************

class DeleteObject : public UpdateOperation {
private:
   string container_name;
   string object_name;

public:
   DeleteObject(StorageSystem* ss, const string& container, const string& object) :
      UpdateOperation(ss, "DeleteObject"),
      container_name(container),
      object_name(object) {
   }

   DeleteObject(const DeleteObject& copy) :
      UpdateOperation(copy),
      container_name(copy.container_name),
      object_name(copy.object_name) {
   }

   DeleteObject& operator=(const DeleteObject& copy) {
      if (this == &copy) {
         return *this;
      }

      UpdateOperation::operator=(copy);

      container_name = copy.container_name;
      object_name = copy.object_name;

      return *this;
   }

   virtual UpdateOperation* clone() {
      return new DeleteObject(*this);
   }

   void run() {
   }
};

//******************************************************************************
//******************************************************************************

//*****************************************************************************

MirrorStorageSystem::MirrorStorageSystem(const string& ini_file_path, bool debug_mode) :
   StorageSystem("Mirror", debug_mode),
   ini_file(ini_file_path),
   primary_ss(NULL),
   secondary_ss(NULL),
   update_in_parallel(false),
   min_updates(0) {
}

//*****************************************************************************

MirrorStorageSystem::~MirrorStorageSystem() {
   exit();
}

//*****************************************************************************

bool MirrorStorageSystem::enter() {
   //TODO:
   return false;
}

//*****************************************************************************

void MirrorStorageSystem::exit() {
   if (primary_ss != NULL) {
      delete primary_ss;
      primary_ss = NULL;
   }

   if (secondary_ss != NULL) {
      delete secondary_ss;
      secondary_ss = NULL;
   }
}

//*****************************************************************************

bool MirrorStorageSystem::have_both_ss() const {
   return (primary_ss != NULL) && (secondary_ss != NULL);
}

//*****************************************************************************

bool MirrorStorageSystem::update(UpdateOperation& update_op) {
   if (have_both_ss()) {

      if (update_in_parallel) {
         update_op.setStorageSystem(primary_ss);
         UpdateOperation* secondary_op = update_op.clone();
         secondary_op->setStorageSystem(secondary_ss);
         PthreadsThread primary_thread(&update_op);
         PthreadsThread secondary_thread(secondary_op);
         primary_thread.setThreadId("primary");
         secondary_thread.setThreadId("secondary");
         bool primary_thread_started = primary_thread.start();
         bool secondary_thread_started = secondary_thread.start();
      } else {
         update_op.setStorageSystem(primary_ss);

      }
   }
   return false;
}

//*****************************************************************************

vector<string> MirrorStorageSystem::list_account_containers() {
   vector<string> list_containers;

   if (have_both_ss()) {
      try {
         list_containers = primary_ss->list_account_containers();
      } catch (exception& e) {
         //TODO: add logging
         try {
            list_containers = secondary_ss->list_account_containers();
         } catch (exception& e) {
            //TODO: add logging
         }
      }
   }

   return list_containers;
}

//*****************************************************************************

bool MirrorStorageSystem::create_container(const string& container_name) {
   bool container_created = false;
   if (have_both_ss()) {
      if (!has_container(container_name)) {
         CreateContainer op(NULL, container_name);
         return update(op);
      }
   }
   return container_created;
}

//*****************************************************************************

bool MirrorStorageSystem::delete_container(const string& container_name) {
   bool container_deleted = false;
   if (have_both_ss()) {
      DeleteContainer op(NULL, container_name);
      return update(op);
   }
   return container_deleted;
}

//*****************************************************************************

vector<string> MirrorStorageSystem::list_container_contents(const string& container_name) {
   vector<string> list_contents;
   if (have_both_ss()) {
      try {
         list_contents = primary_ss->list_container_contents(container_name);
      } catch (exception& e) {
         //TODO: add logging
         try {
            list_contents = secondary_ss->list_container_contents(container_name);
         } catch (exception& e) {
            //TODO: add logging
         }
      }
   }
   return list_contents;
}

//*****************************************************************************

bool MirrorStorageSystem::get_object_metadata(const std::string& container_name,
                                              const std::string& object_name,
                                              PropertySet& dict_props) {
   if (container_name.length() > 0 && object_name.length() > 0) {
      if (have_both_ss()) {
         try {
            if (primary_ss->get_object_metadata(container_name,
                                                object_name,
                                                dict_props)) {
               return true;
            }
         } catch (exception& e) {
            //TODO: add logging
         }

         try {
            if (secondary_ss->get_object_metadata(container_name,
                                                  object_name,
                                                  dict_props)) {
               return true;
            }
         } catch (exception& e) {
            //TODO: add logging
            return false;
         }
      }
   } else {
      if (debug_mode) {
         if (container_name.length() == 0) {
            printf("container name is missing, can't get object metadata\n");
         }
         if (object_name.length() == 0) {
            printf("object name is missing, can't get object metadata\n");
         }
      }
   }
   return false;
}

//*****************************************************************************

bool MirrorStorageSystem::put_object(const string& container_name,
                                     const string& object_name,
                                     const vector<unsigned char>& file_contents,
                                     const PropertySet* headers) {
   bool object_added = false;
   if (container_name.length() > 0 && object_name.length() > 0 && file_contents.size() > 0) {
      if (have_both_ss()) {
         PutObject op(NULL, container_name, object_name);
         object_added = update(op);
      }
   } else {
      if (debug_mode) {
         if (container_name.length() == 0) {
            printf("container name is missing, can't put object\n");
         }
         if (object_name.length() == 0) {
            printf("object name is missing, can't put object\n");
         }
         if (file_contents.size() == 0) {
            printf("object content is empty, can't put object\n");
         }
      }
   }
   return object_added;
}

//*****************************************************************************

bool MirrorStorageSystem::delete_object(const string& container_name,
                                        const string& object_name) {
   bool object_deleted = false;
   if (container_name.length() > 0 && object_name.length() > 0) {
      if (have_both_ss()) {
         DeleteObject op(NULL, container_name, object_name);
         object_deleted = update(op);
      }
   } else {
      if (debug_mode) {
         printf("cannot delete object, container name or object name is missing\n");
      }
   }
   return object_deleted;
}

//*****************************************************************************

int64_t MirrorStorageSystem::get_object(const string& container_name,
                                        const string& object_name,
                                        const string& local_file_path) {
   int64_t bytes_retrieved = 0;

   if (container_name.length() > 0 &&
       object_name.length() > 0 &&
       local_file_path.length() > 0) {

      if (have_both_ss()) {
         try {
            bytes_retrieved = primary_ss->get_object(container_name,
                                                     object_name,
                                                     local_file_path);
            if (bytes_retrieved > 0) {
               return bytes_retrieved;
            }
         } catch (exception& e) {
            //TODO: add logging
         }

         try {
            return secondary_ss->get_object(container_name,
                                            object_name,
                                            local_file_path);
         } catch (exception& e) {
            //TODO: add logging
            return 0;
         }
      }
   } else {
      if (debug_mode) {
         if (container_name.length() == 0) {
            printf("container name is missing, can't get object\n");
         }
         if (object_name.length() == 0) {
            printf("object name is missing, can't get object\n");
         }
         if (local_file_path.size() == 0) {
            printf("local file path is empty, can't get object\n");
         }
      }
   }

   return bytes_retrieved;
}

//*****************************************************************************

