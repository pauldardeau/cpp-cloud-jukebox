#ifndef MIRROR_STORAGE_SYSTEM_H
#define MIRROR_STORAGE_SYSTEM_H

#include <string>
#include <vector>
#include <map>

#include "storage_system.h"
#include "data_types.h"
#include "Runnable.h"


class UpdateOperation : public chaudiere::Runnable {
protected:
   StorageSystem* storage_system;

private:
   std::string op_name;
   bool op_did_run;
   bool op_did_succeed;

public:
   UpdateOperation(const std::string& op);
   UpdateOperation(const UpdateOperation& copy);
   virtual ~UpdateOperation() {}

   UpdateOperation& operator=(const UpdateOperation& copy);

   virtual UpdateOperation* clone() = 0;

   void setStorageSystem(StorageSystem* ss);
   void reset();
   bool did_run() const { return this->op_did_run; }
   bool did_succeed() const { return this->op_did_succeed; }

   virtual void run();
   virtual bool run_operation() = 0;
};


class MirrorStorageSystem : public StorageSystem {
private:
   std::string ini_file;
   StorageSystem* primary_ss;
   StorageSystem* secondary_ss;
   bool update_in_parallel;
   int min_updates;

   MirrorStorageSystem(const MirrorStorageSystem&);
   MirrorStorageSystem& operator=(const MirrorStorageSystem&);

protected:
   bool update(UpdateOperation& update_op);

public:
   MirrorStorageSystem(const std::string& ini_file_path, bool debug_mode = false);
   ~MirrorStorageSystem();

   bool have_both_ss() const;

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

