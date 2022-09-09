#ifndef FILE_METADATA_H
#define FILE_METADATA_H

#include <string>
#include <map>

#include "data_types.h"


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

   void from_dictionary(const std::map<std::string, PropertyValue*>& dictionary,
                        std::string prefix="") {
      std::string key = prefix + "file_uid";
      const auto it_end = dictionary.end();
      auto it = dictionary.find(key);
      if (it != it_end) {
         const PropertyValue* pv = it->second;
	 if (pv->is_string()) {
            file_uid = pv->get_string_value();
	 }
      }

      key = prefix + "file_name";
      it = dictionary.find(key);
      if (it != it_end) {
         const PropertyValue* pv = it->second;
	 if (pv->is_string()) {
            file_name = pv->get_string_value();
         }
      }

      key = prefix + "origin_file_size";
      it = dictionary.find(key);
      if (it != it_end) {
         const PropertyValue* pv = it->second;
	 if (pv->is_ulong()) {
            origin_file_size = pv->get_ulong_value();
	 }
      }

      key = prefix + "stored_file_size";
      it = dictionary.find(key);
      if (it != it_end) {
         const PropertyValue* pv = it->second;
         if (pv->is_ulong()) {
            stored_file_size = pv->get_ulong_value();
	 }
      }

      key = prefix + "pad_char_count";
      it = dictionary.find(key);
      if (it != it_end) {
         const PropertyValue* pv = it->second;
	 if (pv->is_ulong()) {
            pad_char_count = pv->get_ulong_value();
	 }
      }

      key = prefix + "file_time";
      it = dictionary.find(key);
      if (it != it_end) {
         const PropertyValue* pv = it->second;
	 if (pv->is_string()) {
            file_time = pv->get_string_value();
	 }
      }

      key = prefix + "md5_hash";
      it = dictionary.find(key);
      if (it != it_end) {
         const PropertyValue* pv = it->second;
	 if (pv->is_string()) {
            md5_hash = pv->get_string_value();
         }
      }

      key = prefix + "compressed";
      it = dictionary.find(key);
      if (it != it_end) {
         const PropertyValue* pv = it->second;
	 if (pv->is_int()) {
            compressed = pv->get_int_value();
         }
      }

      key = prefix + "encrypted";
      it = dictionary.find(key);
      if (it != it_end) {
         const PropertyValue* pv = it->second;
	 if (pv->is_int()) {
            encrypted = pv->get_int_value();
         }
      }

      key = prefix + "container_name";
      it = dictionary.find(key);
      if (it != it_end) {
         const PropertyValue* pv = it->second;
	 if (pv->is_string()) {
            container_name = pv->get_string_value();
         }
      }

      key = prefix + "object_name";
      it = dictionary.find(key);
      if (it != it_end) {
         const PropertyValue* pv = it->second;
	 if (pv->is_string()) {
            object_name = pv->get_string_value();
         }
      }
   }

   std::map<std::string, PropertyValue*> to_dictionary(std::string prefix = "") const {
      std::map<std::string, PropertyValue*> d;
      d[prefix + "file_uid"] = new StrPropertyValue(file_uid);
      d[prefix + "file_name"] = new StrPropertyValue(file_name);
      d[prefix + "origin_file_size"] = new ULongPropertyValue(origin_file_size);
      d[prefix + "stored_file_size"] = new ULongPropertyValue(stored_file_size);
      d[prefix + "pad_char_count"] = new ULongPropertyValue(pad_char_count);
      d[prefix + "file_time"] = new StrPropertyValue(file_time);
      d[prefix + "md5_hash"] = new StrPropertyValue(md5_hash);
      d[prefix + "compressed"] = new IntPropertyValue(compressed);
      d[prefix + "encrypted"] = new IntPropertyValue(encrypted);
      d[prefix + "container_name"] = new StrPropertyValue(container_name);
      d[prefix + "object_name"] = new StrPropertyValue(object_name);
      return d;
   }
};

#endif

