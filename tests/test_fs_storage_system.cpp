#include <filesystem>

#include "test_fs_storage_system.h"
#include "fs_storage_system.h"
#include "fs_test_case.h"

using namespace std;
using namespace chaudiere;
namespace fs = std::filesystem;

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
   string test_dir = "/tmp/test_cpp_fsstoragesystem_enter";
   fs::remove_all(test_dir);

   {
      FSStorageSystem fs(test_dir, false);
      require(fs.enter(), "enter must return true");
      fs.exit();
   }
   
   {
      FSTestCase fs_test_case(*this, test_dir);
      FSStorageSystem fs(test_dir, false);
      require(fs.enter(), "enter must return true");
      fs.exit();
   }
}

void TestFSStorageSystem::test_exit() {
   TEST_CASE("test_exit");
   string test_dir = "/tmp/test_cpp_fsstoragesystem_exit";
   FSTestCase fs_test_case(*this, test_dir);
   {
      FSStorageSystem fs(test_dir, false);
      fs.exit();
   }
   {
      FSStorageSystem fs(test_dir, false);
      fs.enter();
      fs.exit();
   }
}

void TestFSStorageSystem::test_list_account_containers() {
   TEST_CASE("test_list_account_containers");
   string test_dir = "/tmp/test_cpp_fsstoragesystem_list_account_containers";
   FSTestCase fs_test_case(*this, test_dir);
   FSStorageSystem fs(test_dir, false);
   require(fs.enter(), "enter must return true");

   vector<string> list_containers = fs.list_account_containers();
   require(list_containers.size() == 0, "no containers exist initially");

   require(fs.create_container("moe"), "create container must work");
   require(fs.create_container("larry"), "create container must work");
   require(fs.create_container("curly"), "create container must work");
   list_containers = fs.list_account_containers();
   require(list_containers.size() == 3, "created containers must be listed");
}

void TestFSStorageSystem::test_create_container() {
   TEST_CASE("test_create_container");
   string test_dir = "/tmp/test_cpp_fsstoragesystem_create_container";
   FSTestCase fs_test_case(*this, test_dir);
   FSStorageSystem fs(test_dir, false);
   require(fs.enter(), "enter must return true");

   vector<string> list_containers = fs.list_account_containers();
   require(list_containers.size() == 0, "no containers exist initially");

   require(fs.create_container("foo"), "create container must work");
   list_containers = fs.list_account_containers();
   require(list_containers.size() == 1, "created containers must be listed");

   //TODO: test attempt to call create_container with name of existing
}

void TestFSStorageSystem::test_delete_container() {
   TEST_CASE("test_delete_container");
   string test_dir = "/tmp/test_cpp_fsstoragesystem_delete_container";
   FSTestCase fs_test_case(*this, test_dir);
   FSStorageSystem fs(test_dir, false);
   require(fs.enter(), "enter must return true");
   vector<string> list_containers = fs.list_account_containers();
   require(list_containers.size() == 0, "no containers exist initially");

   require(fs.create_container("foo"), "create container must work");
   list_containers = fs.list_account_containers();
   require(list_containers.size() == 1, "created containers must be listed");

   require(fs.delete_container("foo"), "delete container must return true");
   list_containers = fs.list_account_containers();
   require(list_containers.size() == 0, "delete container must actually delete");
}

void TestFSStorageSystem::test_list_container_contents() {
   TEST_CASE("test_list_container_contents");
   string test_dir = "/tmp/test_cpp_fsstoragesystem_list_container_contents";
   FSTestCase fs_test_case(*this, test_dir);
   FSStorageSystem fs(test_dir, false);
   require(fs.enter(), "enter must return true");
   vector<string> list_containers = fs.list_account_containers();
   require(list_containers.size() == 0, "no containers exist initially");

   // query a non-existing container
   vector<string> list_contents = fs.list_container_contents("bar");
   require(list_contents.size() == 0, "non-existing container must be empty");

   require(fs.create_container("foo"), "create container must work");
   list_contents = fs.list_container_contents("foo");
   require(list_contents.size() == 0, "new container must be empty");
   //TODO: create 1 object and verify in listing
   //TODO: create 2 more objects and verify in listing
}

void TestFSStorageSystem::test_get_object_metadata() {
   TEST_CASE("test_get_object_metadata");
   string test_dir = "/tmp/test_cpp_fsstoragesystem_get_object_metadata";
   FSTestCase fs_test_case(*this, test_dir);
   FSStorageSystem fs(test_dir, false);
   require(fs.enter(), "enter must return true");
   //TODO: non-existing container
   //TODO: existing container, non-existing object
   //TODO: existing container, existing object, no metadata
   //TODO: existing container, existing object, with metadata
   //TODO: existing container, deleted object
   //TODO: implement test_get_object_metadata
}

void TestFSStorageSystem::test_put_object() {
   TEST_CASE("test_put_object");
   string test_dir = "/tmp/test_cpp_fsstoragesystem_put_object";
   FSTestCase fs_test_case(*this, test_dir);
   FSStorageSystem fs(test_dir, false);
   require(fs.enter(), "enter must return true");
   //TODO: non-existing container
   //TODO: existing container, non-existing object
   //TODO: existing container, existing object
   //TODO: implement test_put_object
}

void TestFSStorageSystem::test_delete_object() {
   TEST_CASE("test_delete_object");
   string test_dir = "/tmp/test_cpp_fsstoragesystem_delete_object";
   FSTestCase fs_test_case(*this, test_dir);
   FSStorageSystem fs(test_dir, false);
   require(fs.enter(), "enter must return true");

   // delete object with non-existing container
   requireFalse(fs.delete_object("asdf", "book.txt"), "non existing container should return false");

   // existing container, non-existing object
   require(fs.create_container("books"), "create container should return true");
   requireFalse(fs.delete_object("books", "book.txt"), "non existing object should return false");

   //TODO: existing container, existing object

   //TODO: existing container, just deleted object (double-delete)
}

void TestFSStorageSystem::test_get_object() {
   TEST_CASE("test_get_object");
   string test_dir = "/tmp/test_cpp_fsstoragesystem_get_object";
   FSTestCase fs_test_case(*this, test_dir);
   FSStorageSystem fs(test_dir, false);
   require(fs.enter(), "enter must return true");
   string local_file_path = OSUtils::pathJoin(test_dir, "object.txt");

   // non-existing container
   int ret_val = fs.get_object("asdf", "book.txt", local_file_path);
   require(ret_val == 0, "non-existing container should return 0");

   // existing container, non-existing object
   require(fs.create_container("books"), "create container must return true");
   ret_val = fs.get_object("books", "book.txt", local_file_path);
   require(ret_val == 0, "non-existing object should return 0");

   //TODO: existing container, existing object
   string object_contents = "It was the best of times. It was the worst of times.";
   std::vector<unsigned char> v_obj_contents;
   std::copy(object_contents.begin(), object_contents.end(), std::back_inserter(v_obj_contents));
   require(fs.put_object("books", "book.txt", v_obj_contents, NULL), "put object must work");
   ret_val = fs.get_object("books", "book.txt", local_file_path);
   require(ret_val == object_contents.size(), "existing object should return > 0");
		   
   // existing container, deleted object
   require(fs.delete_object("books", "book.txt"), "delete object must work");
   ret_val = fs.get_object("books", "book.txt", local_file_path);
   require(ret_val == 0, "deleted object should return 0");
}

