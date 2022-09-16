#include "test_fs_storage_system.h"
#include "fs_storage_system.h"

TestFSStorageSystem::TestFSStorageSystem() :
   TestSuite("TestFSStorageSystem") {
}

void TestFSStorageSystem::runTests() {
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

void TestFSStorageSystem::test_enter() {
   TEST_CASE("test_enter");
   //TODO: implement test_enter
}

void TestFSStorageSystem::test_exit() {
   TEST_CASE("test_exit");
   //TODO: implement test_exit
}

void TestFSStorageSystem::test_list_account_containers() {
   TEST_CASE("test_list_account_containers");
   //TODO: implement test_list_account_containers
}

void TestFSStorageSystem::test_create_container() {
   TEST_CASE("test_create_container");
   //TODO: implement test_create_container
}

void TestFSStorageSystem::test_delete_container() {
   TEST_CASE("test_delete_container");
   //TODO: implement test_delete_container
}

void TestFSStorageSystem::test_list_container_contents() {
   TEST_CASE("test_list_container_contents");
   //TODO: implement test_list_container_contents
}

void TestFSStorageSystem::test_get_object_metadata() {
   TEST_CASE("test_get_object_metadata");
   //TODO: implement test_get_object_metadata
}

void TestFSStorageSystem::test_put_object() {
   TEST_CASE("test_put_object");
   //TODO: implement test_put_object
}

void TestFSStorageSystem::test_delete_object() {
   TEST_CASE("test_delete_object");
   //TODO: implement test_delete_object
}

void TestFSStorageSystem::test_get_object() {
   TEST_CASE("test_get_object");
   //TODO: implement test_get_object
}

