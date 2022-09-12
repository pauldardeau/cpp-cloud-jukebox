#include <string.h>
#include "property_set.h"
#include "StringTokenizer.h"

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
   if (pv->is_int()) {
      return pv->get_int_value();
   } else {
      return 0;
   }
}

long PropertySet::get_long_value(const std::string& prop_name) const {
   const PropertyValue* pv = get(prop_name);
   if (pv->is_long()) {
      return pv->get_long_value();
   } else {
      return 0L;
   }
}

unsigned long PropertySet::get_ulong_value(const std::string& prop_name) const {
   const PropertyValue* pv = get(prop_name);
   if (pv->is_ulong()) {
      return pv->get_ulong_value();
   } else {
      return 0L;
   }
}

bool PropertySet::get_bool_value(const std::string& prop_name) const {
   const PropertyValue* pv = get(prop_name);
   if (pv->is_bool()) {
      return pv->get_bool_value();
   } else {
      return false;
   }
}

const std::string& PropertySet::get_string_value(const std::string& prop_name) const {
   const PropertyValue* pv = get(prop_name);
   if (pv->is_string()) {
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
   }
   return success;
}

bool PropertySet::read_from_file(const std::string& file_path) {
   bool success = false;
   FILE* f = fopen(file_path.c_str(), "r");
   if (f != NULL) {
      char* line = NULL;
      size_t len = 0;
      ssize_t read;

      while ((read = getline(&line, &len, f)) != -1) {
         if (strlen(line) > 0) {
            chaudiere::StringTokenizer st(line, "|");
	    int num_tokens = st.countTokens();
	    if (num_tokens == 3) {
               const string& data_type = st.nextToken();
	       const string& key = st.nextToken();
	       const string& value = st.nextToken();
	       if (data_type == TYPE_BOOL) {
                  if (value == VALUE_TRUE) {
                     add(key, new BoolPropertyValue(true)); 
                  } else if (value == VALUE_FALSE) {
                     add(key, new BoolPropertyValue(false));
                  } else {
                     printf("warning: invalid value for bool property (%s), skipping it\n", key.c_str());
                  } 
               } else if (data_type == TYPE_STRING) {
                  add(key, new StrPropertyValue(value));
               } else if (data_type == TYPE_INT) {
                  add(key, new IntPropertyValue(atoi(value.c_str())));
               } else if (data_type == TYPE_LONG) {
                  add(key, new LongPropertyValue(atol(value.c_str())));
               } else if (data_type == TYPE_ULONG) {
                  unsigned long ul_value = (unsigned long) atol(value.c_str());
		  add(key, new ULongPropertyValue(ul_value));
               }
	    }
         }
      }
      free(line);
      fclose(f);
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

