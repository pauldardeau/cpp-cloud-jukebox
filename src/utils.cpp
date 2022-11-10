#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <openssl/buffer.h>
#include <openssl/evp.h>

#include "utils.h"
#include "DateTime.h"
#include "StrUtils.h"

using namespace std;

#define NS_PER_SEC 1000000000.0

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

void Utils::time_sleep_millis(int millis) {
   struct timespec req;
   struct timespec rem;
   req.tv_sec = 0;
   req.tv_nsec = millis * 1000000;
   rem.tv_sec = 0;
   rem.tv_nsec = 0;
   nanosleep(&req, &rem);
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
   return 0 == rmdir(dir_path.c_str());
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

vector<string> Utils::file_read_lines(const string& file_path) {
   string file_text;
   if (file_read_all_text(file_path, file_text)) {
      if (file_text.length() > 0) {
         return chaudiere::StrUtils::split(file_text, "\n");
      }
   }
   return vector<string>();
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

vector<string> Utils::path_split(const string& path) {
   // python: os.path.split (returns tuple of head/tail)
   //
   // split('/home/user/Desktop/file.txt') -> ("/home/user/Desktop/", "file.txt")
   // split('/home/user/Desktop/') -> ("/home/user/Desktop/", "")
   // split('file.txt') -> ("", "file.txt")

   string head = EMPTY;
   string tail = EMPTY;

   if (path.length() > 0) {
      int pos_last_slash = find_last_index(path, '/');
      if (pos_last_slash == -1) {
         // no '/' exists in path (i.e., "file.txt")
         tail = path;
      } else {
         // is '/' the last char in the path?
         if (path[path.length()-1] == '/') {
            head = path;
         } else {
            head = path.substr(0, pos_last_slash + 1);
            tail = path.substr(pos_last_slash + 1);
         }
      }
   }

   vector<string> components;
   components.push_back(head);
   components.push_back(tail);
   return components;
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

   double mtime = 0.0;
   if (Utils::file_get_mtime(path, mtime)) {
      return mtime;
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
      delete [] buffer;
   }
   return success;
}

bool Utils::file_write_all_bytes(const string& file_path,
                                 const vector<unsigned char>& file_bytes) {
   FILE* f = fopen(file_path.c_str(), "wb");
   if (f == NULL) {
      printf("error: file_write_all_bytes unable to open file '%s', errno=%d\n",
             file_path.c_str(), errno);
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

      memset(buffer, 0, sizeof(buffer));

      // fill up the buffer
      for (unsigned long j = 0; j < loop_bytes_to_write; j++) {
         buffer[buff_offset++] = file_bytes[file_offset++];
      }

      size_t items_written = fwrite(buffer, loop_bytes_to_write, 1, f);
      if (items_written < 1) {
         printf("attempted to write 1 item of %ld bytes, wrote %ld items\n",
                loop_bytes_to_write,
                items_written);
         write_success = false;
         break;
      } else {
         num_bytes_to_write -= loop_bytes_to_write;
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

bool Utils::file_get_mtime(const std::string& file_path, double& mtime) {
   struct stat s;
   int rc = stat(file_path.c_str(), &s);
   if (rc == 0) {
      mtime = s.st_mtim.tv_sec;
      mtime += s.st_mtim.tv_nsec / NS_PER_SEC;
      return true;
   } else {
      return false;
   }
}

bool Utils::execute_program(const string& program_path,
                            const vector<string>& program_args,
                            int& exit_code,
                            string& std_out,
                            string& std_err) {
   //TODO: capture stdout and stderr
   bool success = false;
   
   if (program_path.length() == 0) {
      printf("error: a non-empty program_path must be specified\n");
      return false;
   }
   
   if (!file_exists(program_path)) {
      printf("error: program_path '%s' does not exist\n", program_path.c_str());
      return false;
   }
   
   pid_t pid = fork();
   
   if (pid == 0) {
      // child
      vector<string> program_path_components = path_splitext(program_path);
      const string& program_file = program_path_components[1];
      const char* program_name = program_file.c_str();

      // create with extra room for program name and sentinel
      const char **argv = new const char*[program_args.size()+2];
      argv[0] = program_name;  // by convention, argv[0] is program name
      for (unsigned int j = 0; j < program_args.size()+1; ++j) {
         argv[j+1] = program_args[j].c_str();
      }

      argv[program_args.size()+1] = NULL;  // add sentinel
   
      int rc = execv(program_path.c_str(), (char **)argv);
      if (rc == -1) {
         delete [] argv;
      }
   } else {
      // parent
      int status = 0;
      int options = 0;

      pid_t wait_pid = waitpid(pid, &status, options);
      if (wait_pid == pid) {
         if (WIFEXITED(status)) {
            exit_code = WEXITSTATUS(status);
            if (exit_code == 0) {
               success = true;
            }
         } else {
            printf("waitpid returned, but program not exited\n");
         }
      } else {
         printf("waitpid return value (other than pid) = %d\n", wait_pid);
         printf("errno = %d\n", errno);
      }
   }
   
   return success;
}

bool Utils::launch_program(const string& program_path,
                           const vector<string>& program_args,
                           int& child_process_pid) {
   bool success = false;
   
   if (program_path.length() == 0) {
      printf("error: a non-empty program_path must be specified\n");
      return false;
   }
   
   if (!file_exists(program_path)) {
      printf("error: program_path '%s' does not exist\n", program_path.c_str());
      return false;
   }
   
   pid_t pid = fork();
   
   if (pid == 0) {
      // child
      vector<string> program_path_components = path_splitext(program_path);
      const string& program_file = program_path_components[1];
      const char* program_name = program_file.c_str();

      // create with extra room for program name and sentinel
      const char **argv = new const char*[program_args.size()+2];
      argv[0] = program_name;  // by convention, argv[0] is program name
      for (unsigned int j = 0; j < program_args.size()+1; ++j) {
         argv[j+1] = program_args[j].c_str();
      }

      argv[program_args.size()+1] = NULL;  // add sentinel
   
      int rc = execv(program_path.c_str(), (char **)argv);
      if (rc == -1) {
         delete [] argv;
      }
   } else {
      // parent
      child_process_pid = pid;
      success = true;
   }
   
   return success;
}

