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

   requireFalse(fs.create_container("foo"), "call to create container with existing name must return false");
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
   //if (chaudiere::OSUtils::directoryExists(test_dir)) {
   //   std::filesystem::remove_all(test_dir);
   //}

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

   vector<unsigned char> v_obj_contents;
   // create 1 object and verify in listing
   string obj1_contents = "moe\nlarry\ncurly\n";
   auto it = obj1_contents.begin();
   auto it_end = obj1_contents.end();
   for (; it != it_end; it++) {
      v_obj_contents.push_back(*it);
   }
   require(fs.put_object("foo", "stooges.txt", v_obj_contents, NULL), "create object must return true");
   list_contents = fs.list_container_contents("foo");
   require(list_contents.size() == 1, "container must have 1 object");

   // create 2 more objects and verify in listing
   string obj2_contents = "ford\nchevy\ndodge\n";
   v_obj_contents.erase(v_obj_contents.begin(), v_obj_contents.end());
   it = obj2_contents.begin();
   it_end = obj2_contents.end();
   for (; it != it_end; it++) {
      v_obj_contents.push_back(*it);
   }
   require(fs.put_object("foo", "cars.txt", v_obj_contents, NULL), "create object must return true");
   string obj3_contents = "coke\npepsi\nsprite\n";
   v_obj_contents.erase(v_obj_contents.begin(), v_obj_contents.end());
   it = obj3_contents.begin();
   it_end = obj3_contents.end();
   for (; it != it_end; it++) {
      v_obj_contents.push_back(*it);
   }
   require(fs.put_object("foo", "drinks.txt", v_obj_contents, NULL), "create object must return true");
   list_contents = fs.list_container_contents("foo");
   require(list_contents.size() == 3, "container must have 3 objects");
}

void TestFSStorageSystem::test_get_object_metadata() {
   TEST_CASE("test_get_object_metadata");
   string test_dir = "/tmp/test_cpp_fsstoragesystem_get_object_metadata";
   FSTestCase fs_test_case(*this, test_dir);
   //if (chaudiere::OSUtils::directoryExists(test_dir)) {
   //   std::filesystem::remove_all(test_dir);
   //}
   FSStorageSystem fs(test_dir, false);
   require(fs.enter(), "enter must return true");

   // non-existing container
   PropertySet props;
   requireFalse(fs.get_object_metadata("asdf", "foo.txt", props), "get_object_metadata must return false for non-existing container");

   // existing container, non-existing object
   require(fs.create_container("books"), "create container must return true");
   requireFalse(fs.get_object_metadata("books", "book.txt", props), "get_object_metadata must return false for non-existing object");

   // existing container, existing object, no metadata
   vector<unsigned char> v_obj_contents;
   string s = "It was the best of times. It was the worst of times.";
   std::copy(s.begin(), s.end(), std::back_inserter(v_obj_contents));
   require(fs.put_object("books", "book.txt", v_obj_contents, NULL), "put_object must return true for valid args");
   requireFalse(fs.get_object_metadata("books", "book.txt", props), "get_object_metadata must return false for object without metadata");

   // existing container, existing object, with metadata
   PropertySet psPut;
   psPut.add("author", new StrPropertyValue("Charles Dickens"));
   psPut.add("category", new StrPropertyValue("Fiction"));
   psPut.add("subject", new StrPropertyValue("French Revolution"));
   psPut.add("date", new StrPropertyValue("1800s"));
   psPut.add("copyright_expired", new BoolPropertyValue(true));
   require(fs.put_object("books", "tale_of_two_cities.txt", v_obj_contents, &psPut), "put_object with metadata must return true");
   require(fs.get_object_metadata("books", "tale_of_two_cities.txt", props), "get_object_metadata for object with metadata must return true");
   require(props.count() == 5, "retrieved properties must be complete");
   
   // existing container, deleted object
   require(fs.delete_object("books", "tale_of_two_cities.txt"), "delete_object must return true");
   PropertySet psDeleted;
   requireFalse(fs.get_object_metadata("books", "tale_of_two_cities.txt", psDeleted), "get_object_metadata must return false for deleted object");
   require(psDeleted.count() == 0, "no properties must be returned for deleted object");
}

void TestFSStorageSystem::test_put_object() {
   TEST_CASE("test_put_object");
   string test_dir = "/tmp/test_cpp_fsstoragesystem_put_object";
   FSTestCase fs_test_case(*this, test_dir);
   FSStorageSystem fs(test_dir, false);
   require(fs.enter(), "enter must return true");

   // non-existing container
   string object_contents = "It was the best of times. It was the worst of times.";
   std::vector<unsigned char> v_obj_contents;
   std::copy(object_contents.begin(), object_contents.end(), std::back_inserter(v_obj_contents));
   requireFalse(fs.put_object("books", "book.txt", v_obj_contents, NULL), "put object for non-existing container must return false");

   // existing container, non-existing object
   require(fs.create_container("books"), "create container should return true");
   require(fs.put_object("books", "book.txt", v_obj_contents, NULL), "put object for existing container must return true");

   // existing container, existing object
   string upd_object_contents = "Now is the time for all good men to come to the aid of their country.";
   std::vector<unsigned char> v_upd_obj_contents;
   std::copy(upd_object_contents.begin(), upd_object_contents.end(), std::back_inserter(v_upd_obj_contents));
   require(fs.put_object("books", "book.txt", v_upd_obj_contents, NULL), "put object over existing object should return true");
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

   // existing container, existing object
   string object_contents = "It was the best of times. It was the worst of times.";
   std::vector<unsigned char> v_obj_contents;
   std::copy(object_contents.begin(), object_contents.end(), std::back_inserter(v_obj_contents));
   require(fs.put_object("books", "book.txt", v_obj_contents, NULL), "put object must work");
   require(fs.delete_object("books", "book.txt"), "delete of existing object should return true");

   // existing container, just deleted object (double-delete)
   requireFalse(fs.delete_object("books", "book.txt"), "2nd delete of object should return false");
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

   // existing container, existing object
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

