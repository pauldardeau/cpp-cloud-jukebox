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
   static const std::string PROP_CONTENT_LENGTH;
   static const std::string PROP_CONTENT_TYPE;
   static const std::string PROP_CONTENT_MD5;
   static const std::string PROP_CONTENT_ENCODING;
   //static const std::string PROP_CONTENT_DISPOSITION_FILENAME;
   //static const std::string PROP_CACHE_CONTROL;
   //static const std::string PROP_EXPIRES;
   //static const std::string PROP_CANNED_ACL;

   //static const std::string ACL_PRIVATE;
   //static const std::string ACL_PUBLIC_READ;
   //static const std::string ACL_PUBLIC_READ_WRITE;
   //static const std::string ACL_AUTHENTICATED_READ;

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

   // set ACL
   void set_acl_private();
   void set_acl_public_read();
   void set_acl_public_read_write();
   void set_acl_authenticated_read();

   void set_content_encoding(const std::string& content_encoding);
   void set_content_length(unsigned long content_length);
   void set_content_type(const std::string& content_type);
   void set_content_md5(const std::string& md5_hash);

};

#endif

