#ifndef TEST_UTILS_H
#define TEST_UTILS_H

#include <string>
#include "TestSuite.h"


class TestUtils : public poivre::TestSuite {
private:
   std::string test_dir;

protected:
   void runTests();

   void test_datetime_datetime_fromtimestamp();
   void test_time_sleep();
   void test_time_time();
   void test_sys_stdout_write();
   void test_sys_stdout_flush();
   void test_sys_exit();
   void test_path_exists();
   void test_path_isfile();
   void test_find_last_index();
   void test_path_splitext();
   void test_path_getmtime();
   void test_get_pid();
   void test_get_file_size();
   void test_os_rename();
   void test_rename_file();
   void test_file_exists();
   void test_file_append_all_text();
   void test_file_write_all_text();
   void test_file_write_all_bytes();
   void test_file_read_all_bytes();
   void test_file_read_all_text();
   void test_file_read_lines();
   void test_directory_delete_directory();
   void test_md5_for_file();

public:
   TestUtils();
   
};


#endif

