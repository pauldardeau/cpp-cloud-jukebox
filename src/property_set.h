#ifndef PROPERTY_SET_H
#define PROPERTY_SET_H

#include <string>
#include <map>
#include <vector>

#include "data_types.h"


class PropertySet {
private:
   std::map<std::string, PropertyValue*> map_props;

   PropertySet(const PropertySet&);
   PropertySet& operator=(const PropertySet&);

public:
   PropertySet();
   ~PropertySet();

   void add(const std::string& prop_name, PropertyValue* prop_value);
   void clear();

   bool contains(const std::string& prop_name) const;
   void get_keys(std::vector<std::string>& keys) const;
   const PropertyValue* get(const std::string& prop_name) const;

   int get_int_value(const std::string& prop_name) const;
   long get_long_value(const std::string& prop_name) const;
   unsigned long get_ulong_value(const std::string& prop_name) const;
   bool get_bool_value(const std::string& prop_name) const;
   const std::string& get_string_value(const std::string& prop_name) const;

   bool write_to_file(const std::string& file_path) const;
   bool read_from_file(const std::string& file_path);
   size_t count() const;
   std::string to_string() const;
};

#endif

