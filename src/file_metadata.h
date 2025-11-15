#ifndef FILE_METADATA_H
#define FILE_METADATA_H

#include <string>
#include <map>

#include "data_types.h"
#include "property_set.h"


class FileMetadata {
private:
   std::string m_file_uid;
   std::string m_file_name;
   unsigned long m_origin_file_size;
   unsigned long m_stored_file_size;
   unsigned long m_pad_char_count;
   std::string m_file_time;
   std::string m_md5_hash;
   int m_compressed;
   int m_encrypted;
   std::string m_container_name;
   std::string m_object_name;


public:
   FileMetadata() :
      m_origin_file_size(0),
      m_stored_file_size(0),
      m_pad_char_count(0),
      m_compressed(0),
      m_encrypted(0) {
   }

   FileMetadata(const FileMetadata& copy) :
      m_file_uid(copy.m_file_uid),
      m_file_name(copy.m_file_name),
      m_origin_file_size(copy.m_origin_file_size),
      m_stored_file_size(copy.m_stored_file_size),
      m_pad_char_count(copy.m_pad_char_count),
      m_file_time(copy.m_file_time),
      m_md5_hash(copy.m_md5_hash),
      m_compressed(copy.m_compressed),
      m_encrypted(copy.m_encrypted),
      m_container_name(copy.m_container_name),
      m_object_name(copy.m_object_name) {
   }

   ~FileMetadata() {}

   FileMetadata& operator=(const FileMetadata& copy) {
      if (this == &copy) {
         return *this;
      }

      m_file_uid = copy.m_file_uid;
      m_file_name = copy.m_file_name;
      m_origin_file_size = copy.m_origin_file_size;
      m_stored_file_size = copy.m_stored_file_size;
      m_pad_char_count = copy.m_pad_char_count;
      m_file_time = copy.m_file_time;
      m_md5_hash = copy.m_md5_hash;
      m_compressed = copy.m_compressed;
      m_encrypted = copy.m_encrypted;
      m_container_name = copy.m_container_name;
      m_object_name = copy.m_object_name;

      return *this;
   }

   bool operator==(const FileMetadata& other) const {
      if (this == &other) {
         return true;
      }

      return m_file_uid == other.m_file_uid &&
             m_file_name == other.m_file_name &&
             m_origin_file_size == other.m_origin_file_size &&
             m_stored_file_size == other.m_stored_file_size &&
             m_pad_char_count == other.m_pad_char_count &&
             m_file_time == other.m_file_time &&
             m_md5_hash == other.m_md5_hash &&
             m_compressed == other.m_compressed &&
             m_encrypted == other.m_encrypted &&
             m_container_name == other.m_container_name &&
             m_object_name == other.m_object_name;
   }

   bool operator!=(const FileMetadata& other) const {
      return !(this->operator==(other));
   }

   void from_dictionary(const PropertySet& dictionary,
                        std::string prefix="") {
      std::string key = prefix + "file_uid";
      const PropertyValue* pv = dictionary.get(key);
      if (pv != nullptr && pv->is_string()) {
         m_file_uid = pv->get_string_value();
      }

      key = prefix + "file_name";
      pv = dictionary.get(key);
      if (pv != nullptr && pv->is_string()) {
         m_file_name = pv->get_string_value();
      }

      key = prefix + "origin_file_size";
      pv = dictionary.get(key);
      if (pv != nullptr && pv->is_ulong()) {
         m_origin_file_size = pv->get_ulong_value();
      }

      key = prefix + "stored_file_size";
      pv = dictionary.get(key);
      if (pv != nullptr && pv->is_ulong()) {
         m_stored_file_size = pv->get_ulong_value();
      }

      key = prefix + "pad_char_count";
      pv = dictionary.get(key);
      if (pv != nullptr && pv->is_ulong()) {
         m_pad_char_count = pv->get_ulong_value();
      }

      key = prefix + "file_time";
      pv = dictionary.get(key);
      if (pv != nullptr && pv->is_string()) {
         m_file_time = pv->get_string_value();
      }

      key = prefix + "md5_hash";
      pv = dictionary.get(key);
      if (pv != nullptr && pv->is_string()) {
         m_md5_hash = pv->get_string_value();
      }

      key = prefix + "compressed";
      pv = dictionary.get(key);
      if (pv != nullptr && pv->is_int()) {
         m_compressed = pv->get_int_value();
      }

      key = prefix + "encrypted";
      pv = dictionary.get(key);
      if (pv != nullptr && pv->is_int()) {
         m_encrypted = pv->get_int_value();
      }

      key = prefix + "container_name";
      pv = dictionary.get(key);
      if (pv != nullptr && pv->is_string()) {
         m_container_name = pv->get_string_value();
      }

      key = prefix + "object_name";
      pv = dictionary.get(key);
      if (pv != nullptr && pv->is_string()) {
         m_object_name = pv->get_string_value();
      }
   }

   void to_dictionary(PropertySet& d, std::string prefix = "") const {
      d.add(prefix + "file_uid", new StrPropertyValue(m_file_uid));
      d.add(prefix + "file_name", new StrPropertyValue(m_file_name));
      d.add(prefix + "origin_file_size", new ULongPropertyValue(m_origin_file_size));
      d.add(prefix + "stored_file_size", new ULongPropertyValue(m_stored_file_size));
      d.add(prefix + "pad_char_count", new ULongPropertyValue(m_pad_char_count));
      d.add(prefix + "file_time", new StrPropertyValue(m_file_time));
      d.add(prefix + "md5_hash", new StrPropertyValue(m_md5_hash));
      d.add(prefix + "compressed", new IntPropertyValue(m_compressed));
      d.add(prefix + "encrypted", new IntPropertyValue(m_encrypted));
      d.add(prefix + "container_name", new StrPropertyValue(m_container_name));
      d.add(prefix + "object_name", new StrPropertyValue(m_object_name));
   }

   const std::string& get_container_name() const {
      return m_container_name;
   }

   const std::string& get_object_name() const {
      return m_object_name;
   }

   const std::string& get_file_uid() const {
      return m_file_uid;
   }

   const std::string& get_md5_hash() const {
      return m_md5_hash;
   }

   const std::string& get_file_time() const {
      return m_file_time;
   }

   unsigned long get_stored_file_size() const {
      return m_stored_file_size;
   }

   unsigned long get_origin_file_size() const {
      return m_origin_file_size;
   }

   unsigned long get_pad_char_count() const {
      return m_pad_char_count;
   }

   int get_compressed() const {
      return m_compressed;
   }

   int get_encrypted() const {
      return m_encrypted;
   }

   void set_container_name(const std::string& s) {
      m_container_name = s;
   }

   void set_object_name(const std::string& s) {
      m_object_name = s;
   }

   void set_file_uid(const std::string& s) {
      m_file_uid = s;
   }

   void set_file_time(const std::string& s) {
      m_file_time = s;
   }

   void set_md5_hash(const std::string& s) {
      m_md5_hash = s;
   }

   void set_compressed(int i) {
      m_compressed = i;
   }

   void set_encrypted(int i) {
      m_encrypted = i;
   }

   void set_pad_char_count(unsigned long count) {
      m_pad_char_count = count;
   }

   void set_stored_file_size(unsigned long file_size) {
      m_stored_file_size = file_size;
   }

   void set_origin_file_size(unsigned long file_size) {
      m_origin_file_size = file_size;
   }
};

#endif

