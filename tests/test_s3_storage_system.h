#ifndef TEST_S3_STORAGE_SYSTEM_H
#define TEST_S3_STORAGE_SYSTEM_H

#include <string>
#include "TestSuite.h"


class TestS3StorageSystem : public poivre::TestSuite {
protected:
   void runTests();

   void test_enter();
   void test_exit();
   void test_list_account_containers();
   void test_create_container();
   void test_delete_container();
   void test_list_container_contents();
   void test_get_object_metadata();
   void test_put_object();
   void test_delete_object();
   void test_get_object();

public:
   TestS3StorageSystem();
   
};


#endif

