#include <string>
#include <vector>
#include <filesystem>

#include "test_property_set.h"
#include "property_set.h"
#include "utils.h"
#include "StrUtils.h"
#include "OSUtils.h"

using namespace std;

namespace fs = std::filesystem;

TestPropertySet::TestPropertySet() :
   TestSuite("TestPropertySet") {
}

void TestPropertySet::runTests() {
   test_add();
   test_clear();
   test_contains();
   test_get_keys();
   test_get();
   test_get_int_value();
   test_get_long_value();
   test_get_ulong_value();
   test_get_bool_value();
   test_get_string_value();
   test_write_to_file();
   test_read_from_file();
   test_count();
   test_to_string();
}

void TestPropertySet::test_add() {
   TEST_CASE("test_add");
   PropertySet ps;
   ps.add("a_bool", new BoolPropertyValue(true));
   ps.add("an_int", new IntPropertyValue(17));
   ps.add("a_long", new LongPropertyValue(100L));
   ps.add("a_ulong", new ULongPropertyValue(0L));
   ps.add("a_string", new StrPropertyValue("foo"));
}

void TestPropertySet::test_clear() {
   TEST_CASE("test_clear");
   PropertySet ps;
   require(ps.count() == 0);
   ps.clear();
   require(ps.count() == 0);
   ps.add("x", new IntPropertyValue(12));
   ps.add("y", new BoolPropertyValue(true));
   ps.add("z", new StrPropertyValue("zee-value"));
   require(ps.count() == 3);
   ps.clear();
   require(ps.count() == 0);
}

void TestPropertySet::test_contains() {
   TEST_CASE("test_contains");
   PropertySet ps;
   requireFalse(ps.contains("x"), "empty set should return false for contains");
   ps.add("x", new StrPropertyValue("some text"));
   require(ps.contains("x"), "contains return true for added property");
}

void TestPropertySet::test_get_keys() {
   TEST_CASE("test_get_keys");
   PropertySet ps;
   vector<string> keys;
   ps.get_keys(keys);
   require(keys.size() == 0);
   ps.add("stooge1", new StrPropertyValue("Moe"));
   ps.add("stooge2", new StrPropertyValue("Larry"));
   ps.add("stooge3", new StrPropertyValue("Curly"));
   ps.get_keys(keys);
   require(keys.size() == 3);
   string all = "stooge1|stooge2|stooge3";
   for (const auto& key : keys) {
      require(chaudiere::StrUtils::containsString(all, key));
   }
}

void TestPropertySet::test_get() {
   TEST_CASE("test_get");
   PropertySet ps;
   const PropertyValue* pv = ps.get("x");
   require(pv == nullptr);
   ps.add("bool", new BoolPropertyValue(true));
   ps.add("string", new StrPropertyValue("yikes"));
   ps.add("int", new IntPropertyValue(7));
   ps.add("long", new LongPropertyValue(128L));
   ps.add("ulong", new ULongPropertyValue(256L));

   pv = ps.get("bool");
   require(pv != nullptr);
   require(pv->is_bool());
   require(pv->get_bool_value() == true);

   pv = ps.get("string");
   require(pv != nullptr);
   require(pv->is_string());
   requireStringEquals(pv->get_string_value(), "yikes");

   pv = ps.get("int");
   require(pv != nullptr);
   require(pv->is_int());
   require(pv->get_int_value() == 7);

   pv = ps.get("long");
   require(pv != nullptr);
   require(pv->is_long());
   require(pv->get_long_value() == 128L);

   pv = ps.get("ulong");
   require(pv != nullptr);
   require(pv->is_ulong());
   require(pv->get_ulong_value() == 256L);
}

void TestPropertySet::test_get_int_value() {
   TEST_CASE("test_get_int_value");
   PropertySet ps;
   int value = ps.get_int_value("x");
   require(value == 0);
   ps.add("x", new IntPropertyValue(13));
   value = ps.get_int_value("x");
   require(value == 13);
   ps.add("y", new LongPropertyValue(100L));
   value = ps.get_int_value("y");
   require(value == 0);
}

void TestPropertySet::test_get_long_value() {
   TEST_CASE("test_get_long_value");
   PropertySet ps;
   long value = ps.get_long_value("x");
   require(value == 0L);
   ps.add("x", new LongPropertyValue(13L));
   value = ps.get_long_value("x");
   require(value == 13L);
   ps.add("y", new IntPropertyValue(100));
   value = ps.get_long_value("y");
   require(value == 0);
}

void TestPropertySet::test_get_ulong_value() {
   TEST_CASE("test_get_ulong_value");
   PropertySet ps;
   unsigned long value = ps.get_ulong_value("x");
   require(value == 0L);
   ps.add("x", new ULongPropertyValue(13L));
   value = ps.get_ulong_value("x");
   require(value == 13L);
   ps.add("y", new IntPropertyValue(100));
   value = ps.get_ulong_value("y");
   require(value == 0);
}

void TestPropertySet::test_get_bool_value() {
   TEST_CASE("test_get_bool_value");
   PropertySet ps;
   bool value = ps.get_bool_value("x");
   require(value == false);
   ps.add("x", new BoolPropertyValue(true));
   value = ps.get_bool_value("x");
   require(value == true);
   ps.add("y", new IntPropertyValue(2));
   value = ps.get_bool_value("y");
   require(value == false);
}

void TestPropertySet::test_get_string_value() {
   TEST_CASE("test_get_string_value");
   PropertySet ps;
   string value = ps.get_string_value("x");
   requireStringEquals(value, "");
   ps.add("x", new StrPropertyValue("shazam"));
   value = ps.get_string_value("x");
   requireStringEquals(value, "shazam");
   ps.add("y", new IntPropertyValue(32));
   value = ps.get_string_value("y");
   requireStringEquals(value, "");
}

void TestPropertySet::test_write_to_file() {
   TEST_CASE("test_write_to_file");
   string test_dir = "/tmp/test_cpp_property_set_write_to_file";
   chaudiere::OSUtils::createDirectory(test_dir);
   PropertySet ps;
   ps.add("my_bool_true", new BoolPropertyValue(true));
   ps.add("my_bool_false", new BoolPropertyValue(false));
   ps.add("my_int_6", new IntPropertyValue(6));
   ps.add("my_long_150000", new LongPropertyValue(150000L));
   ps.add("my_ulong_2000", new ULongPropertyValue(2000L));
   ps.add("my_string_stars", new StrPropertyValue("stars"));
   string file_name = "property_set.txt";
   string file_path = chaudiere::OSUtils::pathJoin(test_dir, file_name);
   require(ps.write_to_file(file_path), "write_to_file succeeds");
   string exp_file_contents = "bool|my_bool_false|false\n"
                              "bool|my_bool_true|true\n"
                              "int|my_int_6|6\n"
                              "long|my_long_150000|150000\n"
                              "string|my_string_stars|stars\n"
                              "ulong|my_ulong_2000|2000\n";
   string file_contents;
   require(Utils::file_read_all_text(file_path, file_contents), "read file");
   fs::remove_all(test_dir);
   requireStringEquals(exp_file_contents, file_contents);
}

void TestPropertySet::test_read_from_file() {
   TEST_CASE("test_read_from_file");
   string test_dir = "/tmp/test_cpp_property_set_read_from_file";
   string file_name = "property_set.txt";
   string file_path = chaudiere::OSUtils::pathJoin(test_dir, file_name);
   chaudiere::OSUtils::createDirectory(test_dir);
   string file_contents = "bool|my_bool_false|false\n"
                          "bool|my_bool_true|true\n"
                          "int|my_int_6|6\n"
                          "long|my_long_150000|150000\n"
                          "string|my_string_stars|stars\n"
                          "ulong|my_ulong_2000|2000\n";
   require(Utils::file_write_all_text(file_path, file_contents), "write test data to file");

   PropertySet ps;
   bool success = ps.read_from_file(file_path);
   fs::remove_all(test_dir);

   require(success, "read_from_file");
   require(ps.count() == 6, "6 properties");
   require(ps.contains("my_bool_true"), "my_bool_true present");
   require(ps.contains("my_bool_false"), "my_bool_false present");
   require(ps.contains("my_int_6"), "my_int_6 present");
   require(ps.contains("my_long_150000"), "my_long_150000 present");
   require(ps.contains("my_ulong_2000"), "my_ulong_2000 present");
   require(ps.contains("my_string_stars"), "my_string_stars present");

   const PropertyValue* pv = ps.get("my_bool_true");
   if (pv != nullptr) {
      require(pv->is_bool(), "my_bool_true is bool");
      bool bool_value = pv->get_bool_value();
      require(bool_value == true, "my_bool_true value true");
   } else {
      require(pv != nullptr, "my_bool_true property present");
   }

   pv = ps.get("my_bool_false");
   if (pv != nullptr) {
      require(pv->is_bool(), "my_bool_false is bool");
      bool bool_value = pv->get_bool_value();
      require(bool_value == false, "my_bool_false value false");
   } else {
      require(pv != nullptr, "my_bool_false property present");
   }

   pv = ps.get("my_int_6");
   if (pv != nullptr) {
      require(pv->is_int(), "my_int_6 is int");
      int int_value = pv->get_int_value();
      require(int_value == 6, "my_int_6 value = 6");
   } else {
      require(pv != nullptr, "my_int_6 property present");
   }

   pv = ps.get("my_long_150000");
   if (pv != nullptr) {
      require(pv->is_long(), "my_long_150000 is long");
      long long_value = pv->get_long_value();
      require(long_value == 150000L, "my_long_150000 value = 150000L");
   } else {
      require(pv != nullptr, "my_long_150000 property present");
   }

   pv = ps.get("my_ulong_2000");
   if (pv != nullptr) {
      require(pv->is_ulong(), "my_ulong_2000 is ulong");
      unsigned long ul_value = pv->get_ulong_value();
      require(ul_value == 2000L, "my_ulong_2000 value == 2000L");
   } else {
      require(pv != nullptr, "my_ulong_2000 property present");
   }

   pv = ps.get("my_string_stars");
   if (pv != nullptr) {
      require(pv->is_string(), "my_string_stars is string");
      string string_value = pv->get_string_value();
      requireStringEquals(string_value, "stars", "my_string_stars value == stars");
   } else {
      require(pv != nullptr, "my_string_stars property present");
   }
}

void TestPropertySet::test_count() {
   TEST_CASE("test_count");
   PropertySet ps;
   require(ps.count() == 0);
   ps.add("x", new IntPropertyValue(5));
   require(ps.count() == 1);
   ps.add("y", new LongPropertyValue(10));
   ps.add("z", new BoolPropertyValue(true));
   require(ps.count() == 3);
}

void TestPropertySet::test_to_string() {
   TEST_CASE("test_to_string");
   PropertySet ps;
   string value = ps.to_string();
   requireStringEquals(value, "");
   ps.add("x", new IntPropertyValue(5));
   value = ps.to_string();
   requireStringEquals(value, "x");
   ps.add("y", new IntPropertyValue(3));
   ps.add("z", new IntPropertyValue(9));
   value = ps.to_string();
   requireStringEquals(value, "x, y, z");
}

