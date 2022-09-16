#include "test_s3_storage_system.h"

TestS3StorageSystem::TestS3StorageSystem() :
   TestSuite("TestS3StorageSystem") {
}

void TestS3StorageSystem::runTests() {
   test_enter();
   test_exit();
   test_list_account_containers();
   test_create_container();
   test_delete_container();
   test_list_container_contents();
   test_get_object_metadata();
   test_put_object();
   test_delete_object();
   test_get_object();
}

void TestS3StorageSystem::test_enter() {
   TEST_CASE("test_enter");
   //TODO: implement test_enter
}

void TestS3StorageSystem::test_exit() {
   TEST_CASE("test_exit");
   //TODO: implement test_exit
}

void TestS3StorageSystem::test_list_account_containers() {
   TEST_CASE("test_list_account_containers");
   //TODO: implement test_list_account_containers
}

void TestS3StorageSystem::test_create_container() {
   TEST_CASE("test_create_container");
   //TODO: implement test_create_container
}

void TestS3StorageSystem::test_delete_container() {
   TEST_CASE("test_delete_container");
   //TODO: implement test_delete_container
}

void TestS3StorageSystem::test_list_container_contents() {
   TEST_CASE("test_list_container_contents");
   //TODO: implement test_list_container_contents
}

void TestS3StorageSystem::test_get_object_metadata() {
   TEST_CASE("test_get_object_metadata");
   //TODO: implement test_get_object_metadata
}

void TestS3StorageSystem::test_put_object() {
   TEST_CASE("test_put_object");
   //TODO: implement test_put_object
}

void TestS3StorageSystem::test_delete_object() {
   TEST_CASE("test_delete_object");
   //TODO: implement test_delete_object
}

void TestS3StorageSystem::test_get_object() {
   TEST_CASE("test_get_object");
   //TODO: implement test_get_object
}

