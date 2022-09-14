#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <openssl/buffer.h>
#include <openssl/evp.h>

#include "utils.h"
#include "DateTime.h"

using namespace std;

static const string EMPTY = "";

string Utils::datetime_datetime_fromtimestamp(double ts) {
   // python datetime.datetime.fromtimestamp

   //TODO: (1) implement (datetime_datetime_fromtimestamp)
   return EMPTY;
}

void Utils::time_sleep(int seconds) {
   // python time.sleep
   sleep(seconds);
}

double Utils::time_time() {
   // python time.time
   chaudiere::DateTime dt;
   return chaudiere::DateTime::unixTimeValue(dt);
}

void Utils::sys_stdout_write(const string& s) {
   // python sys.stdout.write

   printf("%s", s.c_str());
}
 
void Utils::sys_stdout_flush() {
   // python: sys.stdout.flush

   fflush(stdout);
}

void Utils::sys_exit(int exit_code) {
   // python: sys.exit
   exit(exit_code);
}

bool Utils::path_exists(const string& path) {
   // python: os.path.exists

   struct stat s;
   int rc = stat(path.c_str(), &s);
   if (rc == 0) {
      return (s.st_mode & S_IFREG) || (s.st_mode & S_IFDIR);
   } else {
      return false;
   }
}

bool Utils::file_exists(const string& path) {
   struct stat s;
   int rc = stat(path.c_str(), &s);
   if (rc == 0) {
      return (s.st_mode & S_IFREG);
   } else {
      return false;
   }
}

bool Utils::directory_delete_directory(const std::string& dir_path) {
   int rc = rmdir(dir_path.c_str());
   return rc == 0;
}

bool Utils::file_read_all_text(const string& file_path,
                               string& file_text)
{
   FILE* f = fopen(file_path.c_str(), "rt");
   if (f == NULL) {
      return false;
   }

   bool success = false;

   fseek(f, 0, SEEK_END);
   const long num_file_bytes = ftell(f);
   fseek(f, 0, SEEK_SET);

   if (num_file_bytes > 0L) {
      char* buffer = new char[num_file_bytes+1];
      int num_objects_read = fread(buffer, num_file_bytes, 1, f);
      if (num_objects_read == 1) {
         buffer[num_file_bytes] = '\0';
         file_text = buffer;
         success = true;
      }
      delete [] buffer;
   } else {
      file_text = EMPTY;
      success = true;
   }

   fclose(f);
   return success;
}

bool Utils::path_isfile(const string& path) {
   // python: os.path.isfile

   struct stat s;
   int rc = stat(path.c_str(), &s);
   if (rc == 0) {
      return s.st_mode & S_IFREG;
   } else {
      return false;
   }
}

int Utils::find_last_index(const string& str, char x) {
   int index = -1;
   for (unsigned int i = 0; i < str.length(); i++) {
      if (str[i] == x) {
         index = i;
      }
   }
   return index;
}

vector<string> Utils::path_splitext(const string& path) {
   // python: os.path.splitext

   // splitext("bar") -> ("bar", "")
   // splitext("foo.bar.exe") -> ("foo.bar", ".exe")
   // splitext("/foo/bar.exe") -> ("/foo/bar", ".exe")
   // splitext(".cshrc") -> (".cshrc", "")
   // splitext("/foo/....jpg") -> ("/foo/....jpg", "")

   string root = EMPTY;
   string ext = EMPTY;

   if (path.length() > 0) {
      int pos_last_dot = find_last_index(path, '.');
      if (pos_last_dot == -1) {
         // no '.' exists in path (i.e., "bar")
         root = path;
      } else {
         // is the last '.' the first character? (i.e., ".cshrc")
         if (pos_last_dot == 0) {
            root = path;
         } else {
            char preceding = path[pos_last_dot-1];
            // is the preceding char also a '.'? (i.e., "/foo/....jpg")
            if (preceding == '.') {
               root = path;
            } else {
               // splitext("foo.bar.exe") -> ("foo.bar", ".exe") 
               // splitext("/foo/bar.exe") -> ("/foo/bar", ".exe")
               root = path.substr(0, pos_last_dot);
               ext = path.substr(pos_last_dot);
            }
         }
      }
   }

   vector<string> components;
   components.push_back(root);
   components.push_back(ext);
   return components;
}

double Utils::path_getmtime(const string& path) {
   // python: os.path.getmtime

   struct stat st;
   int rc = stat(path.c_str(), &st);
   if (rc == 0) {
      // struct timespec st_mtim;  // Time of last modification
      //    time_t tv_sec   // whole seconds (valid values are >= 0)
      //    long tv_nsec    // nanoseconds (valid values are [0, 999999999])
      // DQ: should we return any fractional seconds?
      return st.st_mtim.tv_sec * 1.0;
   }

   return 0.0;
}

int Utils::get_pid() {
   // python: os.getpid

   return getpid();
}

long Utils::get_file_size(const string& path_to_file) {
   // python: os.path.getsize

   struct stat st;
   int rc = stat(path_to_file.c_str(), &st);
   if (rc == 0) {
      return st.st_size;
   } else {
      return -1L;
   }
}

bool Utils::os_rename(const string& existing_file,
                      const string& new_file) {
   // python: os.rename
   return rename_file(existing_file, new_file);
}

bool Utils::rename_file(const string& existing_file,
                        const string& new_file) {
   // python: os.rename
   return 0 == rename(existing_file.c_str(), new_file.c_str());
}

bool Utils::file_append_all_text(const string& file_path,
                                 const string& append_file_text) {
   FILE* f = fopen(file_path.c_str(), "a");
   if (f == NULL) {
      return false;
   }

   size_t elems_written = fwrite(append_file_text.c_str(),
                                 append_file_text.length(),
                                 1,
                                 f);
   fclose(f);

   return (elems_written == 1);
}

bool Utils::file_write_all_text(const string& file_path,
                                const string& file_text) {
   FILE* f = fopen(file_path.c_str(), "w");
   if (f == NULL) {
      return false;
   }
   
   size_t elems_written = fwrite(file_text.c_str(),
                                 file_text.length(),
                                 1,
                                 f);
   fclose(f);

   return (elems_written == 1);
}

bool Utils::file_read_all_bytes(const string& file_path,
                                vector<unsigned char>& file_bytes) {
   FILE* f = fopen(file_path.c_str(), "rb");
   if (f == NULL) {
      return false;
   }

   bool success = false;

   fseek(f, 0, SEEK_END);
   const long num_file_bytes = ftell(f);
   fseek(f, 0, SEEK_SET);

   if (num_file_bytes > 0L) {
      unsigned char* buffer = new unsigned char[num_file_bytes];
      int num_objects_read = fread(buffer, num_file_bytes, 1, f);
      if (num_objects_read == 1) {
         for (long l = 0; l < num_file_bytes; l++) {
            file_bytes.push_back(buffer[l]);
         }
         success = true;
      }
   }
   return success;
}

bool Utils::file_write_all_bytes(const string& file_path,
                                 const vector<unsigned char>& file_bytes) {
   FILE* f = fopen(file_path.c_str(), "rb");
   if (f == NULL) {
      return false;
   }

   unsigned long num_bytes_to_write = file_bytes.size();
   unsigned long file_offset = 0;
   unsigned char buffer[4096];
   unsigned long buff_offset = 0;
   unsigned long max_buff_offset = 4095;

   bool write_success = true;

   while (num_bytes_to_write > 0) {
      unsigned long loop_bytes_to_write = max_buff_offset;   
      if (num_bytes_to_write < loop_bytes_to_write) {
         loop_bytes_to_write = num_bytes_to_write;
      }

      buff_offset = 0;

      // fill up the buffer
      for (unsigned long j = 0; j < loop_bytes_to_write; j++) {
         buffer[buff_offset++] = file_bytes[file_offset++];
      }

      size_t items_written = fwrite(buffer, loop_bytes_to_write, 1, f);
      if (items_written < 1) {
         write_success = false;
         break;
      }
   }

   fclose(f);

   return write_success;
}

string Utils::md5_for_file(const string& path_to_file) {
   if (!file_exists(path_to_file)) {
      printf("error (md5_for_file): file does not exist '%s'\n", path_to_file.c_str());
      return EMPTY;
   }

   vector<unsigned char> file_contents;
   if (!file_read_all_bytes(path_to_file, file_contents)) {
      printf("error (md5_for_file): unable to read file '%s'\n", path_to_file.c_str());
      return EMPTY;
   }

   EVP_MD_CTX* ctx = EVP_MD_CTX_create();
   if (ctx == NULL) {
      printf("error (md5_for_file): failed to create EVP_MD_CTX\n");
      return EMPTY;
   }

   if (1 != EVP_DigestInit_ex(ctx, EVP_md5(), NULL)) {
      printf("error (md5_for_file): failed to init MD5 digest\n");
      return EMPTY;
   }

   if (1 != EVP_DigestUpdate(ctx, file_contents.data(), file_contents.size())) {
      printf("error (md5_for_file): failed to update digest\n");
      return EMPTY;
   }

   unsigned int length = EVP_MD_size(EVP_md5());
   unsigned char* digest = (unsigned char*)OPENSSL_malloc(length);
   if (digest == NULL) {
      printf("error (md5_for_file) failed to allocate memory for hash value\n");
      return EMPTY;
   }

   if (1 != EVP_DigestFinal_ex(ctx, digest, &length)) {
      OPENSSL_free(digest);
      printf("error (md5_for_file): failed to finalize digest\n");
      return EMPTY;
   }

   EVP_MD_CTX_destroy(ctx);

   string hash = string((const char*)digest, length);
   OPENSSL_free(digest);

   string hex_formatted_hash;
   char hex_char[3];
   memset(hex_char, 0, sizeof(hex_char));

   for (size_t i = 0; i < hash.length(); i++) {
      sprintf(hex_char, "%02x", hash[i]);
      hex_formatted_hash += hex_char;
   }

   return hex_formatted_hash;
}

