#include <string>
#include <vector>

#include "test_argument_parser.h"
#include "argument_parser.h"
#include "property_set.h"

using namespace std;


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
   ArgumentParser ap;
   ap.addOptionalBoolFlag("--debug", "turn on debugging support");
   vector<string> args;

   PropertySet* ps = ap.parse_args(args);
   require(ps != NULL, "parse_args should return non-NULL");
   if (ps != NULL) {
      requireFalse(ps->contains("debug"), "props must not contain optional argument that wasn't provided");
   }

   args.push_back("--debug");
   ps = ap.parse_args(args);
   require(ps != NULL, "parse_args should return non-NULL");
   if (ps != NULL) {
      require(ps->contains("debug"), "props must contain provided argument");
      require(ps->get_bool_value("debug"), "bool true must be provided for optional bool arg that was provided");
   }
}

void TestArgumentParser::test_add_optional_int_argument() {
   TEST_CASE("test_add_optional_int_argument");
   ArgumentParser ap;
   ap.addOptionalIntArgument("--logLevel", "adjust logging level up or down");

   vector<string> args;
   PropertySet* ps = ap.parse_args(args);
   require(ps != NULL, "parse_args must return non-NULL");
   if (ps != NULL) {
      requireFalse(ps->contains("logLevel"), "property must not exist because it wasn't provided");
   }

   args.push_back("--logLevel");
   args.push_back("5");
   ps = ap.parse_args(args);
   require(ps != NULL, "parse_args must return non-NULL");
   if (ps != NULL) {
      require(ps->contains("logLevel"), "property must exist when provided");
      int value = ps->get_int_value("logLevel");
      require(value == 5, "int value must match what was provided");
   }
}

void TestArgumentParser::test_add_optional_string_argument() {
   TEST_CASE("test_add_optional_string_argument");
   ArgumentParser ap;
   ap.addOptionalStringArgument("--user", "user id for command");

   vector<string> args;
   PropertySet* ps = ap.parse_args(args);
   require(ps != NULL, "parse_args must return non-NULL");
   if (ps != NULL) {
      requireFalse(ps->contains("user"), "property should not exist if not provided");
   }

   args.push_back("--user");
   args.push_back("johndoe");
   ps = ap.parse_args(args);
   require(ps != NULL, "parse_args must return non-NULL");
   if (ps != NULL) {
      require(ps->contains("user"), "provided property must exist");
      string userid = ps->get_string_value("user");
      requireStringEquals("johndoe", userid, "string property values must match");
   }
}

void TestArgumentParser::test_add_required_argument() {
   TEST_CASE("test_add_required_argument");
   ArgumentParser ap;
   ap.addRequiredArgument("command", "command to execute");

   vector<string> args;
   PropertySet* ps = ap.parse_args(args);
   require(ps != NULL, "parse_args must return non-NULL");
   if (ps != NULL) {
      requireFalse(ps->contains("command", "property not provided should not exist");
   }

   args.push_back("play");
   ps = ap.parse_args(args);
   require(ps != NULL, "parse_args must return non-NULL");
   if (ps != NULL) {
      require(ps->contains("command"), "provided property must exist");
      string command = ps->get_string_value("command");
      requireStringEquals("play", command);
   }
}

void TestArgumentParser::test_parse_args() {
   TEST_CASE("test_parse_args");
   ArgumentParser ap;
   ap.addOptionalBoolFlag("--debug", "provide debugging support");
   ap.addOptionalIntArgument("--logLevel", "adjust logging level up or down");
   ap.addOptionalStringArgument("--user", "user issuing command");
   ap.addRequiredArgument("command", "command to execute");

   vector<string> args;
   args.push_back("--logLevel");
   args.push_back("6");
   args.push_back("--user");
   args.push_back("tomjones");
   args.push_back("--debug");
   args.push_back("play");

   PropertySet* ps = ap.parse_args(args);
   require(ps != NULL, "parse_args should return non-NULL");
   if (ps != NULL) {
      require(ps->contains("logLevel"), "logLevel should exist");
      require(ps->contains("user"), "user should exist");
      require(ps->contains("debug"), "debug should exist");
      require(ps->contains("command"), "command should exist");
      int logLevel = ps->get_int_value("logLevel");
      string user = ps->get_string_value("user");
      bool debug = ps->get_bool-value("debug");
      string command = ps->get_string_value("command");
      require(logLevel == 6);
      requireStringEquals("tomjones", user);
      require(debug);
      requireStringEquals("play", command);
   }
}

