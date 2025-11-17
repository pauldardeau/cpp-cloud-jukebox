#ifndef TEST_ARGUMENT_PARSER_H
#define TEST_ARGUMENT_PARSER_H

#include <string>
#include "TestSuite.h"


class TestArgumentParser : public chaudiere::TestSuite {
protected:
   void runTests();

   void test_add_optional_bool_flag();
   void test_add_optional_int_argument();
   void test_add_optional_string_argument();
   void test_add_required_argument();
   void test_parse_args();

public:
   TestArgumentParser();

};


#endif

