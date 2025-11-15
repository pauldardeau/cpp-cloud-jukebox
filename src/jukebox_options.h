#ifndef JUKEBOX_OPTIONS_H
#define JUKEBOX_OPTIONS_H

#include <stdio.h>
#include <string>
#include "utils.h"


class JukeboxOptions {
private:
   bool m_debug_mode;
   bool m_use_encryption;
   bool m_use_compression;
   bool m_check_data_integrity;
   unsigned int m_file_cache_count;
   unsigned int m_number_songs;
   std::string m_encryption_key;
   std::string m_encryption_key_file;
   std::string m_encryption_iv;
   bool m_suppress_metadata_download;


public:
   JukeboxOptions() :
      m_debug_mode(false),
      m_use_encryption(false),
      m_use_compression(false),
      m_check_data_integrity(false),
      m_file_cache_count(3),
      m_number_songs(0),
      m_suppress_metadata_download(false) {
   }

   JukeboxOptions(const JukeboxOptions& copy) :
      m_debug_mode(copy.m_debug_mode),
      m_use_encryption(copy.m_use_encryption),
      m_use_compression(copy.m_use_compression),
      m_check_data_integrity(copy.m_check_data_integrity),
      m_file_cache_count(copy.m_file_cache_count),
      m_number_songs(copy.m_number_songs),
      m_encryption_key(copy.m_encryption_key),
      m_encryption_key_file(copy.m_encryption_key_file),
      m_encryption_iv(copy.m_encryption_iv),
      m_suppress_metadata_download(copy.m_suppress_metadata_download) {
   }

   JukeboxOptions& operator=(const JukeboxOptions& copy) {
      if (this == &copy) {
         return *this;
      }

      m_debug_mode = copy.m_debug_mode;
      m_use_encryption = copy.m_use_encryption;
      m_use_compression = copy.m_use_compression;
      m_check_data_integrity = copy.m_check_data_integrity;
      m_file_cache_count = copy.m_file_cache_count;
      m_number_songs = copy.m_number_songs;
      m_encryption_key = copy.m_encryption_key;
      m_encryption_key_file = copy.m_encryption_key_file;
      m_encryption_iv = copy.m_encryption_iv;
      m_suppress_metadata_download = copy.m_suppress_metadata_download;

      return *this;
   }

   bool validate_options() {
      if (!m_encryption_key_file.empty() &&
          !Utils::file_exists(m_encryption_key_file)) {

         printf("error: encryption key file doesn't exist %s\n",
                m_encryption_key_file.c_str());
         return false;
      }

      if (m_use_encryption) {
         if (m_encryption_key.empty() && m_encryption_key_file.empty()) {
            printf("error: encryption key or encryption key file is required for encryption\n");
            return false;
         }
      }

      return true;
   }

   bool get_debug_mode() const {
      return m_debug_mode;
   }

   bool get_use_encryption() const {
      return m_use_encryption;
   }

   bool get_use_compression() const {
      return m_use_compression;
   }

   bool get_check_data_integrity() const {
      return m_check_data_integrity;
   }

   unsigned int get_file_cache_count() const {
      return m_file_cache_count;
   }

   unsigned int get_number_songs() const {
      return m_number_songs;
   }

   const std::string& get_encryption_key() const {
      return m_encryption_key;
   }

   const std::string& get_encryption_key_file() const {
      return m_encryption_key_file;
   }

   const std::string& get_encryption_iv() const {
      return m_encryption_iv;
   }

   bool get_suppress_metadata_download() const {
      return m_suppress_metadata_download;
   }

   void set_debug_mode(bool b) {
      m_debug_mode = b;
   }

   void set_use_compression(bool b) {
      m_use_compression = b;
   }

   void set_use_encryption(bool b) {
      m_use_encryption = b;
   }

   void set_check_data_integrity(bool b) {
      m_check_data_integrity = b;
   }

   void set_file_cache_count(unsigned int i) {
      m_file_cache_count = i;
   }

   void set_number_songs(unsigned int i) {
      m_number_songs = i;
   }

   void set_encryption_key(const std::string& s) {
      m_encryption_key = s;
   }

   void set_encryption_key_file(const std::string& s) {
      m_encryption_key_file = s;
   }

   void set_encryption_iv(const std::string& s) {
      m_encryption_iv = s;
   }

   void set_suppress_metadata_download(bool b) {
      m_suppress_metadata_download = b;
   }

};

#endif

