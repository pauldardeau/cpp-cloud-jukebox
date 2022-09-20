#include <string.h>
#include "property_set.h"
#include "StringTokenizer.h"
#include "StrUtils.h"
#include "utils.h"

using namespace std;

static const string EMPTY = "";

static const string TYPE_BOOL = "bool";
static const string TYPE_STRING = "string";
static const string TYPE_INT = "int";
static const string TYPE_LONG = "long";
static const string TYPE_ULONG = "ulong";

static const string VALUE_TRUE = "true";
static const string VALUE_FALSE = "false";


PropertySet::PropertySet() {
}

PropertySet::~PropertySet() {
   clear();
}

void PropertySet::add(const std::string& prop_name, PropertyValue* prop_value) {
   if (prop_value != NULL) {
      map_props[prop_name] = prop_value;
   }
}

void PropertySet::clear() {
   auto it = map_props.begin();
   const auto it_end = map_props.end();
   for (; it != it_end; it++) {
      delete it->second;
   }
   map_props.erase(map_props.begin(), map_props.end());
}

bool PropertySet::contains(const std::string& prop_name) const {
   auto it = map_props.find(prop_name);
   return it != map_props.end();
}

void PropertySet::get_keys(std::vector<std::string>& keys) const {
   auto it = map_props.begin();
   const auto it_end = map_props.end();
   for (; it != it_end; it++) {
      keys.push_back(it->first);
   }
}

const PropertyValue* PropertySet::get(const std::string& prop_name) const {
   auto it = map_props.find(prop_name);
   if (it != map_props.end()) {
      return it->second;
   } else {
      return NULL;
   }
}

int PropertySet::get_int_value(const std::string& prop_name) const {
   const PropertyValue* pv = get(prop_name);
   if (pv != NULL && pv->is_int()) {
      return pv->get_int_value();
   } else {
      return 0;
   }
}

long PropertySet::get_long_value(const std::string& prop_name) const {
   const PropertyValue* pv = get(prop_name);
   if (pv != NULL && pv->is_long()) {
      return pv->get_long_value();
   } else {
      return 0L;
   }
}

unsigned long PropertySet::get_ulong_value(const std::string& prop_name) const {
   const PropertyValue* pv = get(prop_name);
   if (pv != NULL && pv->is_ulong()) {
      return pv->get_ulong_value();
   } else {
      return 0L;
   }
}

bool PropertySet::get_bool_value(const std::string& prop_name) const {
   const PropertyValue* pv = get(prop_name);
   if (pv != NULL && pv->is_bool()) {
      return pv->get_bool_value();
   } else {
      return false;
   }
}

const std::string& PropertySet::get_string_value(const std::string& prop_name) const {
   const PropertyValue* pv = get(prop_name);
   if (pv != NULL && pv->is_string()) {
      return pv->get_string_value();
   } else {
      return EMPTY;
   }
}

bool PropertySet::write_to_file(const std::string& file_path) const {
   bool success = false;
   FILE* f = fopen(file_path.c_str(), "w");
   if (f != NULL) {
      auto it = map_props.begin();
      const auto it_end = map_props.end();

      for (; it != it_end; it++) {
         const string& key = it->first;
         const PropertyValue* pv = it->second; 
         if (pv->is_bool()) {
            string value;
            if (pv->get_bool_value()) {
               value = VALUE_TRUE;
            } else {
               value = VALUE_FALSE;
            }
            fprintf(f, "%s|%s|%s\n", TYPE_BOOL.c_str(), key.c_str(), value.c_str());
         } else if (pv->is_string()) {
            fprintf(f, "%s|%s|%s\n", TYPE_STRING.c_str(), key.c_str(), pv->get_string_value().c_str());
         } else if (pv->is_int()) {
            fprintf(f, "%s|%s|%d\n", TYPE_INT.c_str(), key.c_str(), pv->get_int_value());
         } else if (pv->is_long()) {
            fprintf(f, "%s|%s|%ld\n", TYPE_LONG.c_str(), key.c_str(), pv->get_long_value());
         } else if (pv->is_ulong()) {
            fprintf(f, "%s|%s|%lu\n", TYPE_ULONG.c_str(), key.c_str(), pv->get_ulong_value());
         }
      }
      fclose(f);
      success = true;
   }
   return success;
}

bool PropertySet::read_from_file(const std::string& file_path) {
   bool success = false;
   string file_contents;
   if (Utils::file_read_all_text(file_path, file_contents)) {
      if (file_contents.length() > 0) {
         vector<string> file_lines = chaudiere::StrUtils::split(file_contents, "\n");
         auto it = file_lines.begin();
         const auto it_end = file_lines.end();
         for (; it != it_end; it++) {
            const string& file_line = *it;
            string stripped_line = chaudiere::StrUtils::strip(file_line);
            if (stripped_line.length() > 0) {
               vector<string> fields = chaudiere::StrUtils::split(stripped_line, "|");
               if (fields.size() == 3) {
                  const string& data_type = fields[0];
                  const string& prop_name = fields[1];
                  const string& prop_value = fields[2];

                  if (data_type.length() > 0 &&
                      prop_name.length() > 0 &&
                      prop_value.length() > 0) {

                     if (data_type == TYPE_BOOL) {
                        if (prop_value == VALUE_TRUE || prop_value == VALUE_FALSE) {
                           bool bool_value = (prop_value == VALUE_TRUE);
                           add(prop_name, new BoolPropertyValue(bool_value));
                        } else {
                           printf("error: invalid value for type bool '%s'\n", data_type.c_str());
                           printf("skipping\n");
                        }
                     } else if (data_type == TYPE_STRING) {
                        add(prop_name, new StrPropertyValue(prop_value));
                     } else if (data_type == TYPE_INT) {
                        int int_value = chaudiere::StrUtils::parseInt(prop_value);
                        add(prop_name, new IntPropertyValue(int_value));
                     } else if (data_type == TYPE_LONG) {
                        long long_value = chaudiere::StrUtils::parseLong(prop_value);
                        add(prop_name, new LongPropertyValue(long_value));
                     } else if (data_type == TYPE_ULONG) {
                        long long_value = chaudiere::StrUtils::parseLong(prop_value);
                        unsigned long ul_value = (unsigned long) long_value;
                        add(prop_name, new ULongPropertyValue(ul_value));
                     } else {
                        printf("error: unrecognized data type '%s', skipping\n", data_type.c_str());
                     }
                  }
               }
            }
         }
         success = true;
      }
   }
   return success;
}

size_t PropertySet::count() const {
   return map_props.size();
}

std::string PropertySet::to_string() const {
   auto it = map_props.begin();
   const auto it_end = map_props.end();
   bool first_prop = true;
   string props_string;
   const string COMMA_SPACE = ", ";

   for (; it != it_end; it++) {
      if (!first_prop) {
         props_string += COMMA_SPACE;
      }

      props_string += it->first;

      if (first_prop) {
         first_prop = false;
      }
   }

   return props_string;
}

