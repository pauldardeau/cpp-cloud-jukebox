#ifndef JUKEBOX_OPTIONS_H
#define JUKEBOX_OPTIONS_H

#include <stdio.h>
#include <string>
#include "utils.h"


class JukeboxOptions {
public:
   bool debug_mode;
   bool use_encryption;
   bool use_compression;
   bool check_data_integrity;
   int file_cache_count;
   int number_songs;
   std::string encryption_key;
   std::string encryption_key_file;
   std::string encryption_iv;
   bool suppress_metadata_download;

   JukeboxOptions() : 
      debug_mode(false),
      use_encryption(false),
      use_compression(false),
      check_data_integrity(false),
      file_cache_count(3),
      number_songs(0),
      suppress_metadata_download(false) {
   }

   JukeboxOptions(const JukeboxOptions& copy) :
      debug_mode(copy.debug_mode),
      use_encryption(copy.use_encryption),
      use_compression(copy.use_compression),
      check_data_integrity(copy.check_data_integrity),
      file_cache_count(copy.file_cache_count),
      number_songs(copy.number_songs),
      encryption_key(copy.encryption_key),
      encryption_key_file(copy.encryption_key_file),
      encryption_iv(copy.encryption_iv),
      suppress_metadata_download(copy.suppress_metadata_download) {
   }

   JukeboxOptions& operator=(const JukeboxOptions& copy) {
      if (this == &copy) {
         return *this;
      }

      debug_mode = copy.debug_mode;
      use_encryption = copy.use_encryption;
      use_compression = copy.use_compression;
      check_data_integrity = copy.check_data_integrity;
      file_cache_count = copy.file_cache_count;
      number_songs = copy.number_songs;
      encryption_key = copy.encryption_key;
      encryption_key_file = copy.encryption_key_file;
      encryption_iv = copy.encryption_iv;
      suppress_metadata_download = copy.suppress_metadata_download;

      return *this;
   }

   bool validate_options() {
      if (file_cache_count < 0) {
         printf("error: file cache count must be non-negative integer value\n");
         return false;
      }

      if (encryption_key_file.length() > 0 && ! Utils::file_exists(encryption_key_file)) {
         printf("error: encryption key file doesn't exist %s\n", encryption_key_file.c_str());
         return false;
      }

      if (use_encryption) {
         if (encryption_key.length() == 0 && encryption_key_file.length() == 0) {
            printf("error: encryption key or encryption key file is required for encryption\n");
            return false;
         }
      }

      return true;
   }
};

#endif

