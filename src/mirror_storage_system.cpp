#include "mirror_storage_system.h"
#include "utils.h"
#include "OSUtils.h"
#include "Runnable.h"
#include "PthreadsThread.h"

using namespace std;

class CreateContainer;
class DeleteContainer;
class PutObject;
class DeleteObject;

//******************************************************************************
//******************************************************************************

UpdateOperation::UpdateOperation(const string& op) :
   storage_system(NULL),
   op_name(op),
   op_did_run(false),
   op_did_succeed(false) {
}

UpdateOperation::UpdateOperation(const UpdateOperation& copy) :
   storage_system(NULL),
   op_name(copy.op_name),
   op_did_run(false),
   op_did_succeed(false) {
}

UpdateOperation& UpdateOperation::operator=(const UpdateOperation& copy) {
   if (this == &copy) {
      return *this;
   }

   op_name = copy.op_name;
   op_did_run = copy.op_did_run;
   op_did_succeed = copy.op_did_succeed;

   return *this;
}

void UpdateOperation::setStorageSystem(StorageSystem* ss) {
   storage_system = ss;
}

void UpdateOperation::reset() {
   op_did_run = false;
   op_did_succeed = false;
}

void UpdateOperation::run() {
   if (storage_system != NULL) {
      op_did_succeed = run_operation();
   } else {
      printf("error: cannot run UpdateOperation, no storage system set\n");
   }
   op_did_run = true;
}

//******************************************************************************
//******************************************************************************

class CreateContainer : public UpdateOperation {
private:
   string container_name;

public:
   CreateContainer(const string& container) :
      UpdateOperation("CreateContainer"),
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

   bool run_operation() {
      if (storage_system != NULL) {
         return storage_system->create_container(container_name);
      } else {
         return false;
      }
   }
};

//******************************************************************************
//******************************************************************************

class DeleteContainer : public UpdateOperation {
private:
   string container_name;

public:
   DeleteContainer(const string& container) :
      UpdateOperation("DeleteContainer"),
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

   bool run_operation() {
      if (storage_system != NULL) {
         return storage_system->delete_container(container_name);
      } else {
         return false;
      }
   }
};

//******************************************************************************
//******************************************************************************

class PutObject : public UpdateOperation {
private:
   string container_name;
   string object_name;
   const vector<unsigned char>* object_bytes;
   string file_path;
   const PropertySet* headers;

public:
   PutObject(const string& container,
	     const string& object,
	     const vector<unsigned char>* object_contents,
	     const string& object_file_path,
	     const PropertySet* object_headers) :
      UpdateOperation("PutObject"),
      container_name(container),
      object_name(object),
      object_bytes(object_contents),
      file_path(object_file_path),
      headers(object_headers) {
   }

   PutObject(const PutObject& copy) :
      UpdateOperation(copy),
      container_name(copy.container_name),
      object_name(copy.object_name),
      object_bytes(copy.object_bytes),
      file_path(copy.file_path),
      headers(copy.headers) {
   }

   PutObject& operator=(const PutObject& copy) {
      if (this == &copy) {
         return *this;
      }

      UpdateOperation::operator=(copy);

      container_name = copy.container_name;
      object_name = copy.object_name;
      object_bytes = copy.object_bytes;
      file_path = copy.file_path;
      headers = copy.headers;

      return *this;
   }

   virtual UpdateOperation* clone() {
      return new PutObject(*this);
   }

   bool run_operation() {
      if (storage_system != NULL) {
         if (object_bytes != NULL) {
            return storage_system->put_object(container_name,
                                              object_name,
                                              *object_bytes,
                                              headers);
         } else if (file_path.length() > 0) {
            //TODO: implement put_object call with file path
         }
      } 
      return false;
   }
};

//******************************************************************************
//******************************************************************************

class DeleteObject : public UpdateOperation {
private:
   string container_name;
   string object_name;

public:
   DeleteObject(const string& container,
                const string& object) :
      UpdateOperation("DeleteObject"),
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

   bool run_operation() {
      if (storage_system != NULL) {
         return storage_system->delete_object(container_name, object_name);
      } else {
         return false;
      }
   }
};

//******************************************************************************
//******************************************************************************

//*****************************************************************************

MirrorStorageSystem::MirrorStorageSystem(const string& ini_file_path,
                                         bool debug_mode) :
   StorageSystem("Mirror", debug_mode),
   ini_file(ini_file_path),
   primary_ss(NULL),
   secondary_ss(NULL),
   update_in_parallel(false),
   min_updates(1) {
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
      int num_update_successes = 0;
      if (update_in_parallel) {
         update_op.setStorageSystem(primary_ss);
         UpdateOperation* secondary_op = update_op.clone();
         secondary_op->setStorageSystem(secondary_ss);
	 chaudiere::PthreadsThread primary_thread(&update_op);
	 chaudiere::PthreadsThread secondary_thread(secondary_op);
         bool primary_thread_started = primary_thread.start();
         bool secondary_thread_started = secondary_thread.start();
	 if (primary_thread_started) {
            while (!update_op.did_run()) {
               //TODO: add sleep
            }
	    if (update_op.did_succeed()) {
               num_update_successes++;
            }
         }
	 if (secondary_thread_started) {
            while (!secondary_op->did_run()) {
               //TODO: add sleep
            } 
	    if (secondary_op->did_succeed()) {
               num_update_successes++;
            }
         }
	 delete secondary_op;
      } else {
         update_op.setStorageSystem(primary_ss);
	 bool primary_success = update_op.run_operation();
	 if (primary_success) {
            num_update_successes++;
         }
	 update_op.setStorageSystem(secondary_ss);
	 update_op.reset();
	 bool secondary_success = update_op.run_operation();
	 if (secondary_success) {
            num_update_successes++;
         }
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
         CreateContainer op(container_name);
         return update(op);
      }
   }
   return container_created;
}

//*****************************************************************************

bool MirrorStorageSystem::delete_container(const string& container_name) {
   bool container_deleted = false;
   if (have_both_ss()) {
      DeleteContainer op(container_name);
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
   if (container_name.length() > 0 &&
       object_name.length() > 0 &&
       file_contents.size() > 0) {

      if (have_both_ss()) {
         PutObject op(container_name, object_name, &file_contents, "", headers);
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

bool MirrorStorageSystem::put_object_from_file(const string& container_name,
                                               const string& object_name,
                                               const string& object_file_path,
                                               const PropertySet* headers) {
   bool object_added = false;
   if (container_name.length() > 0 &&
       object_name.length() > 0 &&
       object_file_path.length() > 0) {

      if (have_both_ss()) {
         PutObject op(container_name, object_name, NULL, object_file_path, headers);
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
         if (object_file_path.length() == 0) {
            printf("object file path is empty, can't put object\n");
         }
      }
   }
   return object_added;
}

//******************************************************************************

bool MirrorStorageSystem::delete_object(const string& container_name,
                                        const string& object_name) {
   bool object_deleted = false;
   if (container_name.length() > 0 && object_name.length() > 0) {
      if (have_both_ss()) {
         DeleteObject op(container_name, object_name);
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

