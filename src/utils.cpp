#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "utils.h"

using namespace std;


string Utils::datetime_datetime_fromtimestamp(double ts) {
   // python datetime.datetime.fromtimestamp

   //TODO: (1) implement (datetime_datetime_fromtimestamp)
   return string("");
}

void Utils::time_sleep(int seconds) {
   // python time.sleep
   sleep(seconds);
}

double Utils::time_time() {
   // python time.time

   //TODO: implement time_time
   //return DateTime.Now.Subtract(new DateTime(1970, 1, 1)).TotalSeconds;
   return 0.0;
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

bool Utils::file_read_all_text(const string& file_path,
                               string& file_text)
{
   FILE* f = fopen(file_path.c_str(), "r");
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
      file_text = "";
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
   for (int i = 0; i < str.length(); i++) {
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

   string root = "";
   string ext = "";

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

   //TODO: implement path_getmtime
   //DateTime dtModify = File.GetLastWriteTime(path);
   //TimeSpan timeSpan = dtModify - new DateTime(1970, 1, 1, 0, 0, 0);
   //return timeSpan.TotalSeconds;
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

string Utils::md5_for_file(const string& path_to_file) {
   //TODO: implement md5_for_file
   /*
   byte[] file_bytes = File.ReadAllBytes(path_to_file);
   if (file_bytes != null && file_bytes.Length > 0) {
      MD5 md5 = MD5.Create();
      byte[] hash = md5.ComputeHash(file_bytes);
      StringBuilder sb = new StringBuilder();
      for (int i = 0; i < hash.Length; i++) {
         sb.Append(hash[i].ToString("X2"));
      }
      return sb.ToString();
   }
   */

   return string("");
}

