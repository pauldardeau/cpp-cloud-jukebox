#include <memory>

#include "mirror_storage_system.h"
#include "utils.h"
#include "OSUtils.h"
#include "Runnable.h"
#include "PthreadsThread.h"

using namespace std;
using namespace chaudiere;

class CreateContainer;
class DeleteContainer;
class PutObject;
class DeleteObject;

//******************************************************************************
//******************************************************************************

UpdateOperation::UpdateOperation(const string& op) :
   m_storage_system(nullptr),
   m_op_name(op),
   m_op_did_run(false),
   m_op_did_succeed(false) {
}

//*****************************************************************************

UpdateOperation::UpdateOperation(const UpdateOperation& copy) :
   m_storage_system(nullptr),
   m_op_name(copy.m_op_name),
   m_op_did_run(false),
   m_op_did_succeed(false) {
}

//*****************************************************************************

UpdateOperation& UpdateOperation::operator=(const UpdateOperation& copy) {
   if (this == &copy) {
      return *this;
   }

   m_op_name = copy.m_op_name;
   m_op_did_run = copy.m_op_did_run;
   m_op_did_succeed = copy.m_op_did_succeed;

   return *this;
}

//*****************************************************************************

void UpdateOperation::setStorageSystem(StorageSystem* ss) {
   m_storage_system = ss;
}

//*****************************************************************************

void UpdateOperation::reset() {
   m_op_did_run = false;
   m_op_did_succeed = false;
}

//*****************************************************************************

void UpdateOperation::run() {
   if (m_storage_system != nullptr) {
      m_op_did_succeed = run_operation();
   } else {
      printf("error: cannot run UpdateOperation, no storage system set\n");
   }
   m_op_did_run = true;
}

//******************************************************************************
//******************************************************************************

class CreateContainer : public UpdateOperation {
private:
   string m_container_name;

public:
   CreateContainer(const string& container) :
      UpdateOperation("CreateContainer"),
      m_container_name(container) {
   }

   CreateContainer(const CreateContainer& copy) :
      UpdateOperation(copy),
      m_container_name(copy.m_container_name) {
   }

   CreateContainer& operator=(const CreateContainer& copy) {
      if (this == &copy) {
         return *this;
      }

      UpdateOperation::operator=(copy);

      m_container_name = copy.m_container_name;

      return *this;
   }

   virtual UpdateOperation* clone() {
      return new CreateContainer(*this);
   }

   bool run_operation() {
      if (m_storage_system != nullptr) {
         return m_storage_system->create_container(m_container_name);
      } else {
         return false;
      }
   }
};

//******************************************************************************
//******************************************************************************

class DeleteContainer : public UpdateOperation {
private:
   string m_container_name;

public:
   DeleteContainer(const string& container) :
      UpdateOperation("DeleteContainer"),
      m_container_name(container) {
   }

   DeleteContainer(const DeleteContainer& copy) :
      UpdateOperation(copy),
      m_container_name(copy.m_container_name) {
   }

   DeleteContainer& operator=(const DeleteContainer& copy) {
      if (this == &copy) {
         return *this;
      }

      UpdateOperation::operator=(copy);

      m_container_name = copy.m_container_name;

      return *this;
   }

   virtual UpdateOperation* clone() {
      return new DeleteContainer(*this);
   }

   bool run_operation() {
      if (m_storage_system != nullptr) {
         return m_storage_system->delete_container(m_container_name);
      } else {
         return false;
      }
   }
};

//******************************************************************************
//******************************************************************************

class PutObject : public UpdateOperation {
private:
   string m_container_name;
   string m_object_name;
   const vector<unsigned char>* m_object_bytes;
   string m_file_path;
   const PropertySet* m_headers;

public:
   PutObject(const string& container,
             const string& object,
             const vector<unsigned char>* object_contents,
             const string& object_file_path,
             const PropertySet* object_headers) :
      UpdateOperation("PutObject"),
      m_container_name(container),
      m_object_name(object),
      m_object_bytes(object_contents),
      m_file_path(object_file_path),
      m_headers(object_headers) {
   }

   PutObject(const PutObject& copy) :
      UpdateOperation(copy),
      m_container_name(copy.m_container_name),
      m_object_name(copy.m_object_name),
      m_object_bytes(copy.m_object_bytes),
      m_file_path(copy.m_file_path),
      m_headers(copy.m_headers) {
   }

   PutObject& operator=(const PutObject& copy) {
      if (this == &copy) {
         return *this;
      }

      UpdateOperation::operator=(copy);

      m_container_name = copy.m_container_name;
      m_object_name = copy.m_object_name;
      m_object_bytes = copy.m_object_bytes;
      m_file_path = copy.m_file_path;
      m_headers = copy.m_headers;

      return *this;
   }

   virtual UpdateOperation* clone() {
      return new PutObject(*this);
   }

   bool run_operation() {
      if (m_storage_system != nullptr) {
         if (m_object_bytes != nullptr) {
            return m_storage_system->put_object(m_container_name,
                                                m_object_name,
                                                *m_object_bytes,
                                                m_headers);
         } else if (!m_file_path.empty()) {
            return m_storage_system->put_object_from_file(m_container_name,
                                                          m_object_name,
                                                          m_file_path,
                                                          m_headers);
         }
      }
      return false;
   }
};

//******************************************************************************
//******************************************************************************

class DeleteObject : public UpdateOperation {
private:
   string m_container_name;
   string m_object_name;

public:
   DeleteObject(const string& container,
                const string& object) :
      UpdateOperation("DeleteObject"),
      m_container_name(container),
      m_object_name(object) {
   }

   DeleteObject(const DeleteObject& copy) :
      UpdateOperation(copy),
      m_container_name(copy.m_container_name),
      m_object_name(copy.m_object_name) {
   }

   DeleteObject& operator=(const DeleteObject& copy) {
      if (this == &copy) {
         return *this;
      }

      UpdateOperation::operator=(copy);

      m_container_name = copy.m_container_name;
      m_object_name = copy.m_object_name;

      return *this;
   }

   virtual UpdateOperation* clone() {
      return new DeleteObject(*this);
   }

   bool run_operation() {
      if (m_storage_system != nullptr) {
         return m_storage_system->delete_object(m_container_name, m_object_name);
      } else {
         return false;
      }
   }
};

//******************************************************************************
//******************************************************************************

MirrorStorageSystem::MirrorStorageSystem(const string& ini_file_path,
                                         bool debug_mode) :
   StorageSystem("Mirror", debug_mode),
   m_ini_file(ini_file_path),
   m_update_in_parallel(false),
   m_min_updates(1) {
}

//*****************************************************************************

MirrorStorageSystem::~MirrorStorageSystem() {
   MirrorStorageSystem::exit();
}

//*****************************************************************************

bool MirrorStorageSystem::enter() {
   //TODO: implement MirrorStorageSystem::enter
   return false;
}

//*****************************************************************************

void MirrorStorageSystem::exit() {
   if (m_primary_ss) {
      m_primary_ss.reset();
   }

   if (m_secondary_ss) {
      m_secondary_ss.reset();
   }
}

//*****************************************************************************

bool MirrorStorageSystem::have_both_ss() const {
   return m_primary_ss && m_secondary_ss;
}

//*****************************************************************************

bool MirrorStorageSystem::update(UpdateOperation& update_op) {
   if (have_both_ss()) {
      int num_update_successes = 0;
      if (m_update_in_parallel) {
         update_op.setStorageSystem(m_primary_ss.get());
         unique_ptr<UpdateOperation> secondary_op(update_op.clone());
         secondary_op->setStorageSystem(m_secondary_ss.get());
         PthreadsThread primary_thread(&update_op);
         PthreadsThread secondary_thread(secondary_op.get());
         bool primary_thread_started = primary_thread.start();
         bool secondary_thread_started = secondary_thread.start();
         if (primary_thread_started) {
            while (!update_op.did_run()) {
               Utils::time_sleep_millis(200);
            }
            if (update_op.did_succeed()) {
               num_update_successes++;
            }
         }
         if (secondary_thread_started) {
            while (!secondary_op->did_run()) {
               Utils::time_sleep_millis(200);
            }
            if (secondary_op->did_succeed()) {
               num_update_successes++;
            }
         }
      } else {
         update_op.setStorageSystem(m_primary_ss.get());
         bool primary_success = update_op.run_operation();
         if (primary_success) {
            num_update_successes++;
         }
         update_op.setStorageSystem(m_secondary_ss.get());
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
         list_containers = m_primary_ss->list_account_containers();
      } catch (exception& e) {
         printf("MSS::list_account_containers on primary exception: %s\n", e.what());
         try {
            list_containers = m_secondary_ss->list_account_containers();
         } catch (exception& e) {
            printf("MSS::list_account_containers on secondary exception: %s\n", e.what());
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
         list_contents = m_primary_ss->list_container_contents(container_name);
      } catch (exception& e) {
         printf("MSS::list_container_contents exception on primary - %s\n", e.what());
         try {
            list_contents = m_secondary_ss->list_container_contents(container_name);
         } catch (exception& e) {
            printf("MSS::list_container_contents exception on secondary - %s\n", e.what());
         }
      }
   }
   return list_contents;
}

//*****************************************************************************

bool MirrorStorageSystem::get_object_metadata(const std::string& container_name,
                                              const std::string& object_name,
                                              PropertySet& dict_props) {
   if (!container_name.empty() && !object_name.empty()) {
      if (have_both_ss()) {
         try {
            if (m_primary_ss->get_object_metadata(container_name,
                                                  object_name,
                                                  dict_props)) {
               return true;
            }
         } catch (exception& e) {
            printf("MSS::get_object_metadata exception on primary - %s\n", e.what());
         }

         try {
            if (m_secondary_ss->get_object_metadata(container_name,
                                                    object_name,
                                                    dict_props)) {
               return true;
            }
         } catch (exception& e) {
            printf("MSS::get_object_metadata exception on secondary - %s\n", e.what());
            return false;
         }
      }
   } else {
      if (debug_mode()) {
         if (container_name.empty()) {
            printf("container name is missing, can't get object metadata\n");
         }
         if (object_name.empty()) {
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
   if (!container_name.empty() &&
       !object_name.empty() &&
       !file_contents.empty()) {

      if (have_both_ss()) {
         PutObject op(container_name, object_name, &file_contents, "", headers);
         object_added = update(op);
      }
   } else {
      if (debug_mode()) {
         if (container_name.empty()) {
            printf("container name is missing, can't put object\n");
         }
         if (object_name.empty()) {
            printf("object name is missing, can't put object\n");
         }
         if (file_contents.empty()) {
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
   if (!container_name.empty() &&
       !object_name.empty() &&
       !object_file_path.empty()) {

      if (have_both_ss()) {
         PutObject op(container_name, object_name, nullptr, object_file_path, headers);
         object_added = update(op);
      }
   } else {
      if (debug_mode()) {
         if (container_name.empty()) {
            printf("container name is missing, can't put object\n");
         }
         if (object_name.empty()) {
            printf("object name is missing, can't put object\n");
         }
         if (object_file_path.empty()) {
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
   if (!container_name.empty() && !object_name.empty()) {
      if (have_both_ss()) {
         DeleteObject op(container_name, object_name);
         object_deleted = update(op);
      }
   } else {
      if (debug_mode()) {
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

   if (!container_name.empty() &&
       !object_name.empty() &&
       !local_file_path.empty()) {

      if (have_both_ss()) {
         try {
            bytes_retrieved = m_primary_ss->get_object(container_name,
                                                       object_name,
                                                       local_file_path);
            if (bytes_retrieved > 0) {
               return bytes_retrieved;
            }
         } catch (exception& e) {
            printf("MSS::get_object exception on primary - %s\n", e.what());
         }

         try {
            return m_secondary_ss->get_object(container_name,
                                              object_name,
                                              local_file_path);
         } catch (exception& e) {
            printf("MSS::get_object exception on secondary - %s\n", e.what());
            return 0;
         }
      }
   } else {
      if (debug_mode()) {
         if (container_name.empty()) {
            printf("container name is missing, can't get object\n");
         }
         if (object_name.empty()) {
            printf("object name is missing, can't get object\n");
         }
         if (local_file_path.empty()) {
            printf("local file path is empty, can't get object\n");
         }
      }
   }

   return bytes_retrieved;
}

//*****************************************************************************

