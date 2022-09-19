#ifndef FILE_METADATA_H
#define FILE_METADATA_H

#include <string>
#include <map>

#include "data_types.h"
#include "property_set.h"


class FileMetadata {
public:
   std::string file_uid;
   std::string file_name;
   unsigned long origin_file_size;
   unsigned long stored_file_size;
   unsigned long pad_char_count;
   std::string file_time;
   std::string md5_hash;
   int compressed;
   int encrypted;
   std::string container_name;
   std::string object_name;

   FileMetadata() :
      origin_file_size(0),
      stored_file_size(0),
      pad_char_count(0),
      compressed(0),
      encrypted(0) {
   }

   FileMetadata(const FileMetadata& copy) :
      file_uid(copy.file_uid),
      file_name(copy.file_name),
      origin_file_size(copy.origin_file_size),
      stored_file_size(copy.stored_file_size),
      pad_char_count(copy.pad_char_count),
      file_time(copy.file_time),
      md5_hash(copy.md5_hash),
      compressed(copy.compressed),
      encrypted(copy.encrypted),
      container_name(copy.container_name),
      object_name(copy.object_name) {
   }

   ~FileMetadata() {}

   FileMetadata& operator=(const FileMetadata& copy) {
      if (this == &copy) {
         return *this;
      }

      file_uid = copy.file_uid;
      file_name = copy.file_name;
      origin_file_size = copy.origin_file_size;
      stored_file_size = copy.stored_file_size;
      pad_char_count = copy.pad_char_count;
      file_time = copy.file_time;
      md5_hash = copy.md5_hash;
      compressed = copy.compressed;
      encrypted = copy.encrypted;
      container_name = copy.container_name;
      object_name = copy.object_name;

      return *this;
   }

   bool operator==(const FileMetadata& other) const {
      if (this == &other) {
         return true;
      }

      return file_uid == other.file_uid &&
             file_name == other.file_name &&
             origin_file_size == other.origin_file_size &&
             stored_file_size == other.stored_file_size &&
             pad_char_count == other.pad_char_count &&
             file_time == other.file_time &&
             md5_hash == other.md5_hash &&
             compressed == other.compressed &&
             encrypted == other.encrypted &&
             container_name == other.container_name &&
             object_name == other.object_name;
   }

   bool operator!=(const FileMetadata& other) const {
      return !(this->operator==(other));
   }

   void from_dictionary(const PropertySet& dictionary,
                        std::string prefix="") {
      std::string key = prefix + "file_uid";
      const PropertyValue* pv = dictionary.get(key);
      if (pv != NULL && pv->is_string()) {
         file_uid = pv->get_string_value();
      }

      key = prefix + "file_name";
      pv = dictionary.get(key);
      if (pv != NULL && pv->is_string()) {
         file_name = pv->get_string_value();
      }

      key = prefix + "origin_file_size";
      pv = dictionary.get(key);
      if (pv != NULL && pv->is_ulong()) {
         origin_file_size = pv->get_ulong_value();
      }

      key = prefix + "stored_file_size";
      pv = dictionary.get(key);
      if (pv != NULL && pv->is_ulong()) {
         stored_file_size = pv->get_ulong_value();
      }

      key = prefix + "pad_char_count";
      pv = dictionary.get(key);
      if (pv != NULL && pv->is_ulong()) {
         pad_char_count = pv->get_ulong_value();
      }

      key = prefix + "file_time";
      pv = dictionary.get(key);
      if (pv != NULL && pv->is_string()) {
         file_time = pv->get_string_value();
      }

      key = prefix + "md5_hash";
      pv = dictionary.get(key);
      if (pv != NULL && pv->is_string()) {
         md5_hash = pv->get_string_value();
      }

      key = prefix + "compressed";
      pv = dictionary.get(key);
      if (pv != NULL && pv->is_int()) {
         compressed = pv->get_int_value();
      }

      key = prefix + "encrypted";
      pv = dictionary.get(key);
      if (pv != NULL && pv->is_int()) {
         encrypted = pv->get_int_value();
      }

      key = prefix + "container_name";
      pv = dictionary.get(key);
      if (pv != NULL && pv->is_string()) {
         container_name = pv->get_string_value();
      }

      key = prefix + "object_name";
      pv = dictionary.get(key);
      if (pv != NULL && pv->is_string()) {
         object_name = pv->get_string_value();
      }
   }

   void to_dictionary(PropertySet& d, std::string prefix = "") const {
      d.add(prefix + "file_uid", new StrPropertyValue(file_uid));
      d.add(prefix + "file_name", new StrPropertyValue(file_name));
      d.add(prefix + "origin_file_size", new ULongPropertyValue(origin_file_size));
      d.add(prefix + "stored_file_size", new ULongPropertyValue(stored_file_size));
      d.add(prefix + "pad_char_count", new ULongPropertyValue(pad_char_count));
      d.add(prefix + "file_time", new StrPropertyValue(file_time));
      d.add(prefix + "md5_hash", new StrPropertyValue(md5_hash));
      d.add(prefix + "compressed", new IntPropertyValue(compressed));
      d.add(prefix + "encrypted", new IntPropertyValue(encrypted));
      d.add(prefix + "container_name", new StrPropertyValue(container_name));
      d.add(prefix + "object_name", new StrPropertyValue(object_name));
   }
};

#endif

