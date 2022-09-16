#include "test_argument_parser.h"
#include "argument_parser.h"


TestArgumentParser::TestArgumentParser() :
   TestSuite("TestArgumentParser") {
}

void TestArgumentParser::runTests() {
   test_add_optional_bool_flag();
   test_add_optional_int_argument();
   test_add_optional_string_argument();
   test_add_required_argument();
   test_parse_args();
}

void TestArgumentParser::test_add_optional_bool_flag() {
   TEST_CASE("test_add_optional_bool_flag");
   //TODO: implement test_add_optional_bool_flag
}

void TestArgumentParser::test_add_optional_int_argument() {
   TEST_CASE("test_add_optional_int_argument");
   //TODO: implement test_add_optional_int_argument
}

void TestArgumentParser::test_add_optional_string_argument() {
   TEST_CASE("test_add_optional_string_argument");
   //TODO: implement test_add_optional_string_argument
}

void TestArgumentParser::test_add_required_argument() {
   TEST_CASE("test_add_required_argument");
   //TODO: implement test_add_required_argument
}

void TestArgumentParser::test_parse_args() {
   TEST_CASE("test_parse_args");
   //TODO: implement test_parse_args
}

