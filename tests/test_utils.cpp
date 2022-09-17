#include <string>
#include <vector>
#include <filesystem>
#include "test_utils.h"
#include "utils.h"
#include "OSUtils.h"

using namespace std;
using namespace chaudiere;

namespace fs = std::filesystem;

class UtilsTestCase {
private:
   TestUtils& test_suite;
   string test_name;
   string test_dir;

   UtilsTestCase(const UtilsTestCase&);
   UtilsTestCase& operator=(const UtilsTestCase&);
public:
   UtilsTestCase(TestUtils& the_test_suite,
                 const string& the_test_name,
                 const string& the_test_dir);
   ~UtilsTestCase();
   void setup();
   void teardown();
};

UtilsTestCase::UtilsTestCase(TestUtils& the_test_suite,
                             const string& the_test_name,
                             const string& the_test_dir) :
   test_suite(the_test_suite),
   test_name(the_test_name),
   test_dir(the_test_dir) {
   setup();
}

UtilsTestCase::~UtilsTestCase() {
   teardown();
}

void UtilsTestCase::setup() {
   if (OSUtils::directoryExists(test_dir)) {
      fs::remove_all(test_dir);
   }
   test_suite.requireFalse(OSUtils::directoryExists(test_dir), "test dir must not exist");
   Utils::time_sleep_millis(500);
   test_suite.require(OSUtils::createDirectory(test_dir), "create test dir must succeed");
   Utils::time_sleep_millis(500);
   test_suite.require(OSUtils::directoryExists(test_dir), "test dir must exist");
}

void UtilsTestCase::teardown() {
   fs::remove_all(test_dir);
}

//******************************************************************************

TestUtils::TestUtils() :
   TestSuite("TestUtils") {
}

//******************************************************************************

void TestUtils::runTests() {
   test_datetime_datetime_fromtimestamp();
   test_time_sleep();
   test_time_time();
   test_sys_stdout_write();
   test_sys_stdout_flush();
   test_sys_exit();
   test_path_exists();
   test_path_isfile();
   test_find_last_index();
   test_path_splitext();
   test_path_getmtime();
   test_get_pid();
   test_get_file_size();
   test_os_rename();
   test_rename_file();
   test_file_exists();
   test_file_append_all_text();
   test_file_write_all_text();
   test_file_write_all_bytes();
   test_file_read_all_bytes();
   test_file_read_all_text();
   test_file_read_lines();
   test_directory_delete_directory();
   //test_md5_for_file();
}

//******************************************************************************

void TestUtils::test_datetime_datetime_fromtimestamp() {
   //TEST_CASE("test_datetime_datetime_fromtimestamp");
   //TODO: implement test_datetime_datetime_fromtimestamp
   
   //let now = time_time();
   //let now_text = datetime_datetime_fromtimestamp(now);
   //assert!(now_text.len() > 0);
}

//******************************************************************************

void TestUtils::test_time_sleep() {
   //TEST_CASE("test_time_sleep");
   //TODO: implement test_time_sleep
   /*
   let t1 = time_time();
   time_sleep(2);
   let t2 = time_time();
   let delta = t2 - t1;
   assert!(delta > 1.8);
   assert!(delta < 2.2);
   */
}

//******************************************************************************

void TestUtils::test_time_time() {
   //TEST_CASE("test_time_time");
   //TODO: implement test_time_time
   /*
   let now = time_time();
   assert!(now > 0.0);
   */
}

//******************************************************************************

void TestUtils::test_sys_stdout_write() {
   //TEST_CASE("test_sys_stdout_write");
   //TODO: implement test_sys_stdout_write
}

//******************************************************************************

void TestUtils::test_sys_stdout_flush() {
   //TEST_CASE("test_sys_stdout_flush");
   //TODO: implement test_sys_stdout_flush
}

//******************************************************************************

void TestUtils::test_sys_exit() {
   //TEST_CASE("test_sys_exit");
   //TODO: implement test_sys_exit
}

//******************************************************************************

void TestUtils::test_path_exists() {
   TEST_CASE("test_path_exists");
   string test_dir = "/tmp/test_cpp_path_exists";
   UtilsTestCase test(*this, "test_path_exists", test_dir);

   string stooge1_file = OSUtils::pathJoin(test_dir, "moe.txt");
   Utils::file_write_all_text(stooge1_file, "I'm moe\n");
   require(Utils::path_exists(stooge1_file));
}

//******************************************************************************

void TestUtils::test_path_isfile() {
   TEST_CASE("test_path_isfile");
   string test_dir = "/tmp/test_cpp_path_isfile";
   UtilsTestCase test(*this, "test_path_isfile", test_dir);

   string subdir_path = OSUtils::pathJoin(test_dir, "subdir");
   require(OSUtils::createDirectory(subdir_path));
   require(OSUtils::pathExists(subdir_path));
   requireFalse(Utils::path_isfile(subdir_path));

   string stooge1_file = OSUtils::pathJoin(test_dir, "moe.txt");
   Utils::file_write_all_text(stooge1_file, "I'm moe\n");
   require(OSUtils::pathExists(stooge1_file));
   require(Utils::path_isfile(stooge1_file));
}

//******************************************************************************

void TestUtils::test_find_last_index() {
   //TEST_CASE("test_find_last_index");
   //TODO: implement test_find_last_index
}

//******************************************************************************

void TestUtils::test_path_splitext() {
   TEST_CASE("test_path_splitext");
   vector<string> tuple = Utils::path_splitext("bar"); // -> ("bar", "")
   requireStringEquals(tuple[0], "bar");
   requireStringEquals(tuple[1], "");

   tuple = Utils::path_splitext("foo.bar.exe"); // -> ("foo.bar", ".exe")
   requireStringEquals(tuple[0], "foo.bar");
   requireStringEquals(tuple[1], ".exe");

   tuple = Utils::path_splitext("/foo/bar.exe"); // -> ("/foo/bar", ".exe")
   requireStringEquals(tuple[0], "/foo/bar");
   requireStringEquals(tuple[1], ".exe");

   tuple = Utils::path_splitext(".cshrc"); // -> (".cshrc", "")
   requireStringEquals(tuple[0], ".cshrc");
   requireStringEquals(tuple[1], "");

   tuple = Utils::path_splitext("/foo/....jpg"); // -> ("/foo/....jpg", "")
   requireStringEquals(tuple[0], "/foo/....jpg");
   requireStringEquals(tuple[1], "");
}

//******************************************************************************

void TestUtils::test_path_getmtime() {
   TEST_CASE("test_path_getmtime");
   string test_dir = "/tmp/test_cpp_path_getmtime";
   UtilsTestCase test(*this, "test_path_getmtime", test_dir);

   double t1 = Utils::time_time();
   printf("t1 = %f\n", t1);
   require(t1 > 0.0, "t1 > 0.0");
   string stooge1_file = OSUtils::pathJoin(test_dir, "moe.txt");
   Utils::file_write_all_text(stooge1_file, "I'm moe\n");
   //time_sleep_millis(200);
   double t2 = Utils::time_time();
   printf("t2 = %f\n", t2);
   require(t2 > 0.0, "t2 > 0.0");
   Utils::file_append_all_text(stooge1_file, "and I'm joe\n");
   double t3 = Utils::time_time();
   printf("t3 = %f\n", t3);
   require(t3 > 0.0, "t3 > 0.0");
   require(t2 > t1, "t2 > t1");
   require(t3 > t2, "t3 > t2");
   double delta = t3 - t2;
   require(delta < 2.0, "delta < 2.0");
}

//******************************************************************************

void TestUtils::test_get_pid() {
   TEST_CASE("test_get_pid");
   int first_pid = Utils::get_pid();
   require(first_pid > 0);
   int second_pid = Utils::get_pid();
   require(second_pid > 0);
   require(first_pid == second_pid);
}

//******************************************************************************

void TestUtils::test_get_file_size() {
   string test_dir = "/tmp/test_cpp_get_file_size";
   UtilsTestCase test(*this, "test_get_file_size", test_dir);
   TEST_CASE("test_get_file_size");

   string stooge1_file = OSUtils::pathJoin(test_dir, "moe.txt");
   Utils::file_write_all_text(stooge1_file, "I'm moe\n");
   long file_size = Utils::get_file_size(stooge1_file);
   require(file_size == 8);
}

//******************************************************************************

void TestUtils::test_os_rename() {
   string test_dir = "/tmp/test_cpp_os_rename";

   UtilsTestCase test(*this, "test_os_rename", test_dir);
   TEST_CASE("test_os_rename");

   string stooge1_file = OSUtils::pathJoin(test_dir, "moe.txt");
   string stooge2_file = OSUtils::pathJoin(test_dir, "larry.txt");
   Utils::file_write_all_text(stooge1_file, "I'm moe\n");
   require(Utils::os_rename(stooge1_file, stooge2_file), "os_rename returns true");
   vector<string> list_files = OSUtils::listFilesInDirectory(test_dir);
   require(list_files.size() == 1, "dir listing return 1 file");
   requireStringEquals(list_files[0], "larry.txt", "file name should match");
}

//******************************************************************************

void TestUtils::test_rename_file() {
   string test_dir = "/tmp/test_cpp_rename_file";
   UtilsTestCase test(*this, "test_rename_file", test_dir);
   TEST_CASE("test_rename_file");

   string stooge1_file = OSUtils::pathJoin(test_dir, "moe.txt");
   string stooge2_file = OSUtils::pathJoin(test_dir, "larry.txt");
   Utils::file_write_all_text(stooge1_file, "I'm moe\n");
   require(Utils::rename_file(stooge1_file, stooge2_file));
   vector<string> list_files = OSUtils::listFilesInDirectory(test_dir);
   require(list_files.size() == 1);
   requireStringEquals(list_files[0], "larry.txt");
}

//******************************************************************************

void TestUtils::test_file_exists() {
   TEST_CASE("test_file_exists");
   string test_dir = "/tmp/test_cpp_file_exists";
   UtilsTestCase test(*this, "test_file_exists", test_dir);

   string stooge1_file = OSUtils::pathJoin(test_dir, "moe.txt");
   requireFalse(Utils::file_exists(stooge1_file));
   Utils::file_write_all_text(stooge1_file, "I'm moe\n");
   require(Utils::file_exists(stooge1_file));
}

//******************************************************************************

void TestUtils::test_file_append_all_text() {
   string test_dir = "/tmp/test_cpp_file_append_all_text";
   UtilsTestCase test(*this, "test_file_append_all_text", test_dir);
   TEST_CASE("test_file_append_all_text");

   string file_path = OSUtils::pathJoin(test_dir, "autos.txt");
   require(Utils::file_write_all_text(file_path,
                                    "Ford\nChevy\n"));
   require(Utils::file_append_all_text(file_path,
                                     "Nissan\nToyota\n"));
   vector<string> file_lines = Utils::file_read_lines(file_path);
   require(file_lines.size() > 3);
   requireStringEquals(file_lines[0], "Ford");
   requireStringEquals(file_lines[1], "Chevy");
   requireStringEquals(file_lines[2], "Nissan");
   requireStringEquals(file_lines[3], "Toyota");
}

//******************************************************************************

void TestUtils::test_file_write_all_text() {
   string test_dir = "/tmp/test_cpp_file_write_all_text";
   UtilsTestCase test(*this, "test_file_write_all_text", test_dir);
   TEST_CASE("test_file_write_all_text");

   string file_path = OSUtils::pathJoin(test_dir, "autos.txt");
   require(Utils::file_write_all_text(file_path,
                                    "Ford\nChevy\nNissan\nToyota"));
   vector<string> file_lines = Utils::file_read_lines(file_path);
   require(file_lines.size() > 3);
   requireStringEquals(file_lines[0], "Ford");
   requireStringEquals(file_lines[1], "Chevy");
   requireStringEquals(file_lines[2], "Nissan");
   requireStringEquals(file_lines[3], "Toyota");
}

//******************************************************************************

void TestUtils::test_file_write_all_bytes() {
   string test_dir = "/tmp/test_cpp_file_write_all_bytes";

   UtilsTestCase test(*this, "test_file_write_all_bytes", test_dir);
   TEST_CASE("test_file_write_all_bytes");

   string file_contents = "Ford\nChevy\nNissan\nToyota";
   vector<unsigned char> v_file_contents;
   std::copy(file_contents.begin(),
             file_contents.end(),
             std::back_inserter(v_file_contents));
   string file_path = OSUtils::pathJoin(test_dir, "autos.txt");
   require(Utils::file_write_all_bytes(file_path, v_file_contents));
   vector<string> file_lines = Utils::file_read_lines(file_path);
   require(file_lines.size() > 3);
   requireStringEquals(file_lines[0], "Ford");
   requireStringEquals(file_lines[1], "Chevy");
   requireStringEquals(file_lines[2], "Nissan");
   requireStringEquals(file_lines[3], "Toyota");
}

//******************************************************************************

void TestUtils::test_file_read_all_bytes() {
   string test_dir = "/tmp/test_cpp_file_read_all_bytes";
   UtilsTestCase test(*this, "test_file_read_all_bytes", test_dir);
   TEST_CASE("test_file_read_all_bytes");

   string file_path = OSUtils::pathJoin(test_dir, "stooges.txt");
   require(Utils::file_write_all_text(file_path, "Moe,Larry,Curly,Shemp,Joe"));
   vector<unsigned char> file_bytes;
   bool read_success = Utils::file_read_all_bytes(file_path, file_bytes);
   require(read_success);
   require(file_bytes.size() > 0);
   require(file_bytes.size() == 25);
}

//******************************************************************************

void TestUtils::test_file_read_all_text() {
   string test_dir = "/tmp/test_cpp_file_read_all_text";
   UtilsTestCase test(*this, "test_file_read_all_text", test_dir);

   TEST_CASE("test_file_read_all_text");
   string stooge1_file = OSUtils::pathJoin(test_dir, "moe.txt");

   Utils::file_write_all_text(stooge1_file, "I'm moe\n");
   require(Utils::file_exists(stooge1_file), "file exists");
   string file_contents;
   bool read_success = Utils::file_read_all_text(stooge1_file, file_contents);
   require(read_success, "read succeeds");
   requireStringEquals(file_contents, "I'm moe\n", "matching file contents");
}

//******************************************************************************

void TestUtils::test_file_read_lines() {
}

//******************************************************************************

void TestUtils::test_directory_delete_directory() {
   //TEST_CASE("test_directory_delete_directory");
   //TODO: implement test_directory_delete_directory
}

//******************************************************************************

void TestUtils::test_md5_for_file() {
   string test_dir = "/tmp/test_cpp_md5_for_file";
   UtilsTestCase test(*this, "test_md5_for_file", test_dir);
   TEST_CASE("test_md5_for_file");

   string file_path = OSUtils::pathJoin(test_dir, "stooges.txt");
   bool write_success = Utils::file_write_all_text(file_path, "moe\nlarry\ncurly\nshemp\njoe\n");
   require(write_success, "write succeeds");
   string md5_hash = Utils::md5_for_file(file_path);
   require(md5_hash.length() > 0, "md5 hash is not empty");
   requireStringEquals(md5_hash, "172f966fe02ff84c0f36178fa7aaa686", "matching md5 hashes");
}

//******************************************************************************

