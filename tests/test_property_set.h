#ifndef TEST_PROPERTY_SET_H
#define TEST_PROPERTY_SET_H

#include <string>
#include "TestSuite.h"


class TestPropertySet : public chaudiere::TestSuite {
private:
   std::string test_dir;

protected:
   void runTests();

   void test_add();
   void test_clear();

   void test_contains();
   void test_get_keys();
   void test_get();

   void test_get_int_value();
   void test_get_long_value();
   void test_get_ulong_value();
   void test_get_bool_value();
   void test_get_string_value();
   void test_write_to_file();
   void test_read_from_file();
   void test_count();
   void test_to_string();

public:
   TestPropertySet();
   
};


#endif

