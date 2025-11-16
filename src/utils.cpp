#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <errno.h>
#include <math.h>
#include <memory>
#include <time.h>
#include <sys/select.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include "utils.h"
#include "DateTime.h"
#include "StrUtils.h"
#include "IniReader.h"
#include "KeyValuePairs.h"
#include "OSUtils.h"


using namespace std;
using namespace chaudiere;

#define READ_PIPE 0
#define WRITE_PIPE 1

#define NS_PER_SEC 1000000000.0

static bool finished_reading_child_fds = false;
static const string EMPTY = "";

//*****************************************************************************

void sig_child_handler(int signo) {
   if (signo == SIGCHLD) {
      finished_reading_child_fds = true;
      wait(nullptr);
   }
}

//*****************************************************************************

string Utils::datetime_datetime_fromtimestamp(double ts) {
   // python datetime.datetime.fromtimestamp

   //TODO: (1) implement (datetime_datetime_fromtimestamp)
   return EMPTY;
}

//*****************************************************************************

void Utils::time_sleep(int seconds) {
   // python time.sleep
   sleep(seconds);
}

//*****************************************************************************

void Utils::time_sleep_millis(int millis) {
   struct timespec req;
   struct timespec rem;
   req.tv_sec = 0;
   req.tv_nsec = millis * 1000000;
   rem.tv_sec = 0;
   rem.tv_nsec = 0;
   nanosleep(&req, &rem);
}

//*****************************************************************************

double Utils::time_time() {
   // python time.time
   DateTime dt;
   return DateTime::unixTimeValue(dt);
}

//*****************************************************************************

void Utils::sys_stdout_write(const string& s) {
   // python sys.stdout.write

   printf("%s", s.c_str());
}

//*****************************************************************************

void Utils::sys_stdout_flush() {
   // python: sys.stdout.flush

   fflush(stdout);
}

//*****************************************************************************

void Utils::sys_exit(int exit_code) {
   // python: sys.exit
   exit(exit_code);
}

//*****************************************************************************

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

//*****************************************************************************

bool Utils::file_exists(const string& path) {
   struct stat s;
   int rc = stat(path.c_str(), &s);
   if (rc == 0) {
      return (s.st_mode & S_IFREG);
   } else {
      return false;
   }
}

//*****************************************************************************

bool Utils::directory_delete_directory(const std::string& dir_path) {
   return 0 == rmdir(dir_path.c_str());
}

//*****************************************************************************

bool Utils::file_read_all_text(const string& file_path,
                               string& file_text)
{
   FILE* f = fopen(file_path.c_str(), "rt");
   if (f == nullptr) {
      return false;
   }

   bool success = false;

   fseek(f, 0, SEEK_END);
   const long num_file_bytes = ftell(f);
   fseek(f, 0, SEEK_SET);

   if (num_file_bytes > 0L) {
      char* buffer(new char[num_file_bytes+1]);
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

//*****************************************************************************

vector<string> Utils::file_read_lines(const string& file_path) {
   string file_text;
   if (file_read_all_text(file_path, file_text)) {
      if (!file_text.empty()) {
         return StrUtils::split(file_text, "\n");
      }
   }
   return vector<string>();
}

//*****************************************************************************

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

//*****************************************************************************

int Utils::find_last_index(const string& str, char x) {
   int index = -1;
   for (unsigned int i = 0; i < str.length(); i++) {
      if (str[i] == x) {
         index = i;
      }
   }
   return index;
}

//*****************************************************************************

vector<string> Utils::path_split(const string& path) {
   // python: os.path.split (returns tuple of head/tail)
   //
   // split('/home/user/Desktop/file.txt') -> ("/home/user/Desktop/", "file.txt")
   // split('/home/user/Desktop/') -> ("/home/user/Desktop/", "")
   // split('file.txt') -> ("", "file.txt")

   string head = EMPTY;
   string tail = EMPTY;

   if (!path.empty()) {
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

   vector<string> components(2);
   components.push_back(head);
   components.push_back(tail);
   return components;
}

//*****************************************************************************

void Utils::path_splitext(const string& path,
                          std::vector<std::string>& tuple) {
   // python: os.path.splitext

   // splitext("bar") -> ("bar", "")
   // splitext("foo.bar.exe") -> ("foo.bar", ".exe")
   // splitext("/foo/bar.exe") -> ("/foo/bar", ".exe")
   // splitext(".cshrc") -> (".cshrc", "")
   // splitext("/foo/....jpg") -> ("/foo/....jpg", "")

   string root = EMPTY;
   string ext = EMPTY;

   if (!path.empty()) {
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

   tuple.clear();
   tuple.push_back(root);
   tuple.push_back(ext);
}

//*****************************************************************************

double Utils::path_getmtime(const string& path) {
   // python: os.path.getmtime

   double mtime = 0.0;
   if (Utils::file_get_mtime(path, mtime)) {
      return mtime;
   }

   return 0.0;
}

//*****************************************************************************

int Utils::get_pid() {
   // python: os.getpid

   return getpid();
}

//*****************************************************************************

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

//*****************************************************************************

bool Utils::os_rename(const string& existing_file,
                      const string& new_file) {
   // python: os.rename
   return rename_file(existing_file, new_file);
}

//*****************************************************************************

bool Utils::rename_file(const string& existing_file,
                        const string& new_file) {
   // python: os.rename
   return 0 == rename(existing_file.c_str(), new_file.c_str());
}

//*****************************************************************************

bool Utils::file_append_all_text(const string& file_path,
                                 const string& append_file_text) {
   FILE* f = fopen(file_path.c_str(), "a");
   if (f == nullptr) {
      return false;
   }

   size_t elems_written = fwrite(append_file_text.c_str(),
                                 append_file_text.length(),
                                 1,
                                 f);
   fclose(f);

   return (elems_written == 1);
}

//*****************************************************************************

bool Utils::file_write_all_text(const string& file_path,
                                const string& file_text) {
   FILE* f = fopen(file_path.c_str(), "w");
   if (f == nullptr) {
      return false;
   }

   size_t elems_written = fwrite(file_text.c_str(),
                                 file_text.length(),
                                 1,
                                 f);
   fclose(f);

   return (elems_written == 1);
}

//*****************************************************************************

bool Utils::file_read_all_bytes(const string& file_path,
                                vector<unsigned char>& file_bytes) {
   FILE* f = fopen(file_path.c_str(), "rb");
   if (f == nullptr) {
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

//*****************************************************************************

bool Utils::file_write_all_bytes(const string& file_path,
                                 const vector<unsigned char>& file_bytes) {
   FILE* f = fopen(file_path.c_str(), "wb");
   if (f == nullptr) {
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

//*****************************************************************************

string Utils::md5_for_file(const string& ini_file_name,
                           const string& path_to_file) {
   if (!file_exists(ini_file_name)) {
      printf("error (md5_for_file): ini file does not exist '%s'\n", ini_file_name.c_str());
      return EMPTY;
   }

   if (!file_exists(path_to_file)) {
      printf("error (md5_for_file): file does not exist '%s'\n", path_to_file.c_str());
      return EMPTY;
   }

   KeyValuePairs kvp;
   if (get_platform_config_values(ini_file_name, kvp)) {
      string key_exe = "md5_exe_file_name";
      string key_field_number = "md5_hash_output_field";
      if (kvp.hasKey(key_exe)) {
         string md5_exe = kvp.getValue(key_exe);
         if (!file_exists(md5_exe)) {
            printf("error: md5 executable not found: '%s'\n", md5_exe.c_str());
            return EMPTY;
         }

         vector<string> program_args;
         program_args.push_back(path_to_file);
         int exit_code = 0;
         string std_out;
         string std_err;

         if (execute_program(md5_exe,
                             program_args,
                             exit_code,
                             std_out,
                             std_err)) {
            if (exit_code == 0) {
               if (!std_out.empty()) {
                  int field_number = 1;
                  if (kvp.hasKey(key_field_number)) {
                     string field_number_text = kvp.getValue(key_field_number);
                     if (!field_number_text.empty()) {
                        try {
                           field_number = StrUtils::parseInt(field_number_text);
                        } catch (const exception& e) {
                           printf("error: unable to convert value '%s' for '%s' to integer\n",
                                  field_number_text.c_str(),
                                  key_field_number.c_str());
                           printf("will attempt to use first field\n");
                        }
                     }
                  }
                  vector<string> file_lines = StrUtils::split(std_out, "\n");
                  if (!file_lines.empty()) {
                     string first_line = file_lines[0];
                     vector<string> line_fields = StrUtils::split(first_line, " ");
                     if (!line_fields.empty()) {
                        return line_fields[field_number-1];
                     } else {
                        if (!first_line.empty()) {
                           return first_line;
                        } else {
                           printf("error: md5_for_file - first stdout line is empty\n");
                        }
                     }
                  } else {
                     printf("error: md5_for_file - stdout split by lines is empty\n");
                  }
               } else {
                  printf("error: md5_for_file - no content for stdout captured\n");
               }
            } else {
               printf("error: md5_for_file - non-zero exit code for md5 utility. value=%d\n", exit_code);
            }
         } else {
            printf("error: md5_for_file - unable to execute md5 sum utility '%s'\n", md5_exe.c_str());
         }
      } else {
         printf("error: md5_for_file - no value present for '%s'\n", key_exe.c_str());
      }
   } else {
      printf("error: md5_for_file - unable to retrieve platform config values\n");
   }

   return EMPTY;
}

//*****************************************************************************

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

//*****************************************************************************

bool Utils::file_delete(const string& file_path) {
   return OSUtils::deleteFile(file_path);
}

//*****************************************************************************

bool Utils::file_copy(const string& from_file, const string& to_file) {
   if (!file_exists(from_file)) {
      return false;
   }

   long file_size = get_file_size(from_file);

   FILE* f_source = fopen(from_file.c_str(), "r");
   if (f_source == nullptr) {
      return false;
   }

   FILE* f_dest = fopen(to_file.c_str(), "w");
   if (f_dest == nullptr) {
      fclose(f_source);
      return false;
   }

   long bytes_remaining = file_size;
   size_t bytes_to_copy = 4096;
   size_t bytes_transferred;
   unsigned char byte_buffer[4096];

   while (bytes_remaining > 0) {
      if (bytes_remaining < 4096) {
         bytes_to_copy = bytes_remaining;
      }
      bytes_transferred = fread(byte_buffer,
                                1,
                                bytes_to_copy,
                                f_source);
      if (bytes_transferred < bytes_to_copy) {
         fclose(f_source);
         fclose(f_dest);
         file_delete(to_file);
         return false;
      } else {
         bytes_transferred = fwrite(byte_buffer,
                                    1,
                                    bytes_to_copy,
                                    f_dest);
         if (bytes_transferred < bytes_to_copy) {
            fclose(f_source);
            fclose(f_dest);
            file_delete(to_file);
            return false;
         } else {
            bytes_remaining -= bytes_to_copy;
         }
      }
   }

   fclose(f_source);
   fclose(f_dest);

   return true;
}

//*****************************************************************************

bool Utils::file_set_permissions(const string& file_path,
                                 int user_perms,
                                 int group_perms,
                                 int world_perms) {
   bool success = false;
   if (file_exists(file_path)) {
      if ((user_perms > -1) && (group_perms > -1) && (world_perms > -1) &&
          (user_perms < 8) && (group_perms < 8) && (world_perms < 8)) {
         mode_t file_mode = 0;
         if (user_perms > 3) {
            file_mode |= S_IRUSR;
            user_perms -= 4;
         }
         if (user_perms > 1) {
            file_mode |= S_IWUSR;
            user_perms -= 2;
         }
         if (user_perms > 0) {
            file_mode |= S_IXUSR;
         }

         if (group_perms > 3) {
            file_mode |= S_IRGRP;
            group_perms -= 4;
         }
         if (group_perms > 1) {
            file_mode |= S_IWGRP;
            group_perms -= 2;
         }
         if (group_perms > 0) {
            file_mode |= S_IXGRP;
         }

         if (world_perms > 3) {
            file_mode |= S_IROTH;
            world_perms -= 4;
         }
         if (world_perms > 1) {
            file_mode |= S_IWOTH;
            world_perms -= 2;
         }
         if (world_perms > 0) {
            file_mode |= S_IXOTH;
         }

         int rc = chmod(file_path.c_str(), file_mode);
         if (rc == 0) {
            success = true;
         } else {
            printf("Utils::file_set_permissions - chmod failed for '%s', errno = %d\n", file_path.c_str(), errno);
         }
      } else {
         printf("Utils::file_set_permissions - error permission flags are octal (0-7)\n");
      }
   } else {
      printf("Utils::file_set_permissions - file doesn't exist '%s'\n", file_path.c_str());
   }
   return success;
}

//*****************************************************************************

bool Utils::execute_program(const string& program_path,
                            const vector<string>& program_args,
                            int& exit_code,
                            string& std_out,
                            string& std_err) {
   bool success = false;

   if (program_path.empty()) {
      printf("error: a non-empty program_path must be specified\n");
      return false;
   }

   if (!file_exists(program_path)) {
      printf("error: program_path '%s' does not exist\n", program_path.c_str());
      return false;
   }

   int fd_stdout[2];
   int fd_stderr[2];
   int rc;

   rc = pipe(fd_stdout);
   if (rc != 0) {
      printf("error: unable to create pipe. errno = %d\n", errno);
      return false;
   }

   rc = pipe(fd_stderr);
   if (rc != 0) {
      printf("error: unable to create pipe. errno = %d\n", errno);
      return false;
   }

   pid_t pid = fork();

   if (pid == 0) {
      // child
      dup2(fd_stdout[WRITE_PIPE], 1);
      dup2(fd_stderr[WRITE_PIPE], 2);

      close(fd_stdout[READ_PIPE]);
      close(fd_stdout[WRITE_PIPE]);

      close(fd_stderr[READ_PIPE]);
      close(fd_stderr[WRITE_PIPE]);

      vector<string> program_path_components;
      path_splitext(program_path, program_path_components);
      const string& program_file = program_path_components[1];
      const char* program_name = program_file.c_str();

      // create with extra room for program name and sentinel
      const char **argv = new const char*[program_args.size()+2];
      argv[0] = program_name;  // by convention, argv[0] is program name
      for (unsigned int j = 0; j < program_args.size()+1; ++j) {
         argv[j+1] = program_args[j].c_str();
      }

      argv[program_args.size()+1] = nullptr;  // add sentinel

      int rc = execv(program_path.c_str(), (char **)argv);
      if (rc == -1) {
         delete [] argv;
         string s = "error: unable to start executable";
         ssize_t exp_bytes_written = (ssize_t) s.length();
         ssize_t bytes_written = write(2, s.c_str(), exp_bytes_written);
         if (bytes_written < exp_bytes_written) {
            printf("error: unable to start executable\n");
            printf("error: unable to write to stderr\n");
         }
         exit(1);
      }
   } else {
      // parent

      close(fd_stdout[WRITE_PIPE]);
      close(fd_stderr[WRITE_PIPE]);

      char pipe_read_buffer[8192];
      memset(pipe_read_buffer, 0, sizeof(pipe_read_buffer));

      fd_set read_fds;

      signal(SIGCHLD, sig_child_handler);

      do {
         FD_ZERO(&read_fds);
         FD_SET(fd_stdout[READ_PIPE], &read_fds);
         FD_SET(fd_stderr[READ_PIPE], &read_fds);
         int cnt = select(std::max(fd_stdout[READ_PIPE],
                                   fd_stderr[READ_PIPE]) + 1,  // nfds
                          &read_fds,                           // readfds
                          nullptr,                             // writefds
                          nullptr,                             // exceptfds
                          nullptr);                            // timeout
         if (cnt > 0) {
            if (FD_ISSET(fd_stdout[READ_PIPE], &read_fds)) {
               ssize_t bytes_read = read(fd_stdout[READ_PIPE],
                                         pipe_read_buffer,
                                         sizeof(pipe_read_buffer)-1);
               if (bytes_read < 0) {
                  printf("error: unable to read pipe. errno = %d\n", errno);
               } else if (bytes_read > 0) {
                  if (pipe_read_buffer[0] != 0) {
                     std_out += string(pipe_read_buffer);
                  }
                  memset(pipe_read_buffer, 0, sizeof(pipe_read_buffer));
               }
            }

            if (FD_ISSET(fd_stderr[READ_PIPE], &read_fds)) {
               ssize_t bytes_read = read(fd_stderr[READ_PIPE],
                                         pipe_read_buffer,
                                         sizeof(pipe_read_buffer)-1);
               if (bytes_read < 0) {
                  printf("error: unable to read pipe. errno = %d\n", errno);
               } else if (bytes_read > 0) {
                  if (pipe_read_buffer[0] != 0) {
                     std_err += string(pipe_read_buffer);
                  }
                  memset(pipe_read_buffer, 0, sizeof(pipe_read_buffer));
               }
            }
         } else if (cnt < 0) {
            printf("error on select: errno = %d\n", errno);
            finished_reading_child_fds = true;
         }
      } while (!finished_reading_child_fds);
      close(fd_stdout[READ_PIPE]);
      close(fd_stderr[READ_PIPE]);

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
         if (errno == ECHILD) {
            success = true;
         } else {
            printf("waitpid error. errno = %d\n", errno);
         }
      }
   }

   return success;
}

//*****************************************************************************

bool Utils::launch_program(const string& program_path,
                           const vector<string>& program_args,
                           int& child_process_pid) {
   bool success = false;

   if (program_path.empty()) {
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
      vector<string> program_path_components;
      path_splitext(program_path, program_path_components);
      const string& program_file = program_path_components[1];
      const char* program_name = program_file.c_str();

      // create with extra room for program name and sentinel
      const char **argv = new const char*[program_args.size()+2];
      argv[0] = program_name;  // by convention, argv[0] is program name
      for (unsigned int j = 0; j < program_args.size()+1; ++j) {
         argv[j+1] = program_args[j].c_str();
      }

      argv[program_args.size()+1] = nullptr;  // add sentinel

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

//*****************************************************************************

string Utils::get_platform_identifier() {
   string os_identifier;

#if defined(__APPLE__)
   os_identifier = "mac";
#elif defined(__linux__)
   os_identifier = "linux";
#elif defined(__FreeBSD__)
   os_identifier = "freebsd";
#elif defined(__unix__)
   os_identifier = "unix";
#elif defined(_WIN32)
   os_identifier = "windows";
#else
   os_identifier = "unknown";
#endif

   return os_identifier;
}

//*****************************************************************************

bool Utils::get_platform_config_value(const string& ini_file_name,
                                      const string& key,
                                      string& config_value,
                                      bool strip_quotes) {
   string os_identifier = Utils::get_platform_identifier();
   if (os_identifier == "unknown" || os_identifier.empty()) {
      printf("error: unknown platform\n");
      return false;
   }

   KeyValuePairs kvp;
   if (get_platform_config_values(ini_file_name, kvp)) {
      if (kvp.hasKey(key)) {
         config_value = kvp.getValue(key);
         if (StrUtils::startsWith(config_value, "\"") &&
             StrUtils::endsWith(config_value, "\"")) {
            StrUtils::strip(config_value, '"');
         }
         StrUtils::strip(config_value);

         return true;
      } else {
         printf("error: %s missing value for '%s' within [%s]\n",
                ini_file_name.c_str(),
                key.c_str(),
                os_identifier.c_str());
         return false;
      }
   } else {
      printf("error: unable to obtain config values\n");
      return false;
   }
}

//*****************************************************************************

bool Utils::get_platform_config_values(const string& ini_file_name,
                                       KeyValuePairs& kvp) {
   string os_identifier = Utils::get_platform_identifier();
   if (os_identifier == "unknown" || os_identifier.empty()) {
      printf("error: unknown platform\n");
      return false;
   }

   try {
      IniReader ini_reader(ini_file_name);
      if (!ini_reader.readSection(os_identifier, kvp)) {
         printf("error: no config section present for '%s'\n", os_identifier.c_str());
         return false;
      } else {
         return true;
      }
   } catch (const exception& e) {
      printf("error: unable to read %s - %s\n", ini_file_name.c_str(), e.what());
      return false;
   }
}

//*****************************************************************************

