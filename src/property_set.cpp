#include <string.h>
#include "property_set.h"
#include "StrUtils.h"
#include "utils.h"

using namespace std;
using namespace chaudiere;

static const string EMPTY = "";

static const string TYPE_BOOL = "bool";
static const string TYPE_STRING = "string";
static const string TYPE_INT = "int";
static const string TYPE_LONG = "long";
static const string TYPE_ULONG = "ulong";

static const string VALUE_TRUE = "true";
static const string VALUE_FALSE = "false";

const string PropertySet::PROP_CONTENT_ENCODING = "Content-Encoding";
const string PropertySet::PROP_CONTENT_LENGTH = "Content-Length";
const string PropertySet::PROP_CONTENT_TYPE = "Content-Type";
const string PropertySet::PROP_CONTENT_MD5 = "Content-MD5";
//const string PropertySet::PROP_CONTENT_DISPOSITION_FILENAME = "contentDispositionFilename";
//const string PropertySet::PROP_CACHE_CONTROL = "Cache-Control";
//const string PropertySet::PROP_EXPIRES = "expires";
//const string PropertySet::PROP_CANNED_ACL = "cannedAcl";

//const string PropertySet::ACL_PRIVATE = "private";
//const string PropertySet::ACL_PUBLIC_READ = "public-read";
//const string PropertySet::ACL_PUBLIC_READ_WRITE = "public-read-write";
//const string PropertySet::ACL_AUTHENTICATED_READ = "authenticated-read";


//*****************************************************************************

PropertySet::PropertySet() {
}

//*****************************************************************************

PropertySet::~PropertySet() {
   clear();
}

//*****************************************************************************

void PropertySet::add(const std::string& prop_name, PropertyValue* prop_value) {
   if (prop_value != nullptr) {
      m_map_props[prop_name] = prop_value;
   }
}

//*****************************************************************************

void PropertySet::clear() {
   for (auto& pair : m_map_props) {
      delete pair.second;
   }
   m_map_props.erase(m_map_props.begin(), m_map_props.end());
}

//*****************************************************************************

bool PropertySet::contains(const std::string& prop_name) const {
   auto it = m_map_props.find(prop_name);
   return it != m_map_props.end();
}

//*****************************************************************************

void PropertySet::get_keys(std::vector<std::string>& keys) const {
   for (const auto& pair : m_map_props) {
      keys.push_back(pair.first);
   }
}

//*****************************************************************************

const PropertyValue* PropertySet::get(const std::string& prop_name) const {
   auto it = m_map_props.find(prop_name);
   if (it != m_map_props.end()) {
      return it->second;
   } else {
      return nullptr;
   }
}

//*****************************************************************************

int PropertySet::get_int_value(const std::string& prop_name) const {
   const PropertyValue* pv = get(prop_name);
   if (pv != nullptr && pv->is_int()) {
      return pv->get_int_value();
   } else {
      return 0;
   }
}

//*****************************************************************************

long PropertySet::get_long_value(const std::string& prop_name) const {
   const PropertyValue* pv = get(prop_name);
   if (pv != nullptr && pv->is_long()) {
      return pv->get_long_value();
   } else {
      return 0L;
   }
}

//*****************************************************************************

unsigned long PropertySet::get_ulong_value(const std::string& prop_name) const {
   const PropertyValue* pv = get(prop_name);
   if (pv != nullptr && pv->is_ulong()) {
      return pv->get_ulong_value();
   } else {
      return 0L;
   }
}

//*****************************************************************************

bool PropertySet::get_bool_value(const std::string& prop_name) const {
   const PropertyValue* pv = get(prop_name);
   if (pv != nullptr && pv->is_bool()) {
      return pv->get_bool_value();
   } else {
      return false;
   }
}

//*****************************************************************************

const std::string& PropertySet::get_string_value(const std::string& prop_name) const {
   const PropertyValue* pv = get(prop_name);
   if (pv != nullptr && pv->is_string()) {
      return pv->get_string_value();
   } else {
      return EMPTY;
   }
}

//*****************************************************************************

bool PropertySet::write_to_file(const std::string& file_path) const {
   bool success = false;
   FILE* f = fopen(file_path.c_str(), "w");
   if (f != nullptr) {
      for (const auto& pair : m_map_props) {
         const string& key = pair.first;
         const PropertyValue* pv = pair.second;
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

//*****************************************************************************

bool PropertySet::read_from_file(const std::string& file_path) {
   bool success = false;
   string file_contents;
   if (Utils::file_read_all_text(file_path, file_contents)) {
      if (!file_contents.empty()) {
         vector<string> file_lines = StrUtils::split(file_contents, "\n");

         for (const auto& file_line : file_lines) {
            string stripped_line = StrUtils::strip(file_line);
            if (!stripped_line.empty()) {
               vector<string> fields = StrUtils::split(stripped_line, "|");
               if (fields.size() == 3) {
                  const string& data_type = fields[0];
                  const string& prop_name = fields[1];
                  const string& prop_value = fields[2];

                  if (!data_type.empty() &&
                      !prop_name.empty() &&
                      !prop_value.empty()) {

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
                        int int_value = StrUtils::parseInt(prop_value);
                        add(prop_name, new IntPropertyValue(int_value));
                     } else if (data_type == TYPE_LONG) {
                        long long_value = StrUtils::parseLong(prop_value);
                        add(prop_name, new LongPropertyValue(long_value));
                     } else if (data_type == TYPE_ULONG) {
                        long long_value = StrUtils::parseLong(prop_value);
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

//*****************************************************************************

size_t PropertySet::count() const {
   return m_map_props.size();
}

//*****************************************************************************

std::string PropertySet::to_string() const {
   bool first_prop = true;
   string props_string;
   const string COMMA_SPACE = ", ";

   for (const auto& pair : m_map_props) {
      if (!first_prop) {
         props_string += COMMA_SPACE;
      }

      props_string += pair.first;

      if (first_prop) {
         first_prop = false;
      }
   }

   return props_string;
}

//*****************************************************************************

/*
void PropertySet::set_acl_private() {
   add(PROP_CANNED_ACL, new StrPropertyValue(ACL_PRIVATE));
}

//*****************************************************************************

void PropertySet::set_acl_public_read() {
   add(PROP_CANNED_ACL, new StrPropertyValue(ACL_PUBLIC_READ));
}

//*****************************************************************************

void PropertySet::set_acl_public_read_write() {
   add(PROP_CANNED_ACL, new StrPropertyValue(ACL_PUBLIC_READ_WRITE));
}

//*****************************************************************************

void PropertySet::set_acl_authenticated_read() {
   add(PROP_CANNED_ACL, new StrPropertyValue(ACL_AUTHENTICATED_READ));
}
*/

//*****************************************************************************

void PropertySet::set_content_encoding(const std::string& content_encoding) {
   add(PROP_CONTENT_ENCODING, new StrPropertyValue(content_encoding));
}

//*****************************************************************************

void PropertySet::set_content_length(unsigned long content_length) {
   add(PROP_CONTENT_LENGTH, new ULongPropertyValue(content_length));
}

//*****************************************************************************

void PropertySet::set_content_type(const std::string& content_type) {
   add(PROP_CONTENT_TYPE, new StrPropertyValue(content_type));
}

//*****************************************************************************

void PropertySet::set_content_md5(const std::string& md5_hash) {
   add(PROP_CONTENT_MD5, new StrPropertyValue(md5_hash));
}

//*****************************************************************************

