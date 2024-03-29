#ifndef UTILS_H
#define UTILS_H

#include <string>
#include <vector>

#include "KeyValuePairs.h"

//namespace jukebox {

class Utils {
public:
   static std::string datetime_datetime_fromtimestamp(double ts);
   static void time_sleep(int seconds);
   static void time_sleep_millis(int millis);
   static double time_time();
   static void sys_stdout_write(const std::string& s);
   static void sys_stdout_flush();
   static void sys_exit(int exit_code);
   static bool path_exists(const std::string& path);
   static bool path_isfile(const std::string& path);
   static int find_last_index(const std::string& str, char x);
   static std::vector<std::string> path_split(const std::string& path);
   static std::vector<std::string> path_splitext(const std::string& path);
   static double path_getmtime(const std::string& path);
   static int get_pid();
   static long get_file_size(const std::string& path_to_file);
   static bool os_rename(const std::string& existing_file,
                         const std::string& new_file);
   static bool rename_file(const std::string& existing_file,
                           const std::string& new_file);
   static bool file_delete(const std::string& file_path);
   static bool file_exists(const std::string& file_path);
   static bool file_append_all_text(const std::string& file_path,
                                    const std::string& new_content);
   static bool file_write_all_text(const std::string& file_path,
                                   const std::string& file_content);
   static bool file_write_all_bytes(const std::string& file_path,
                                    const std::vector<unsigned char>& file_bytes);
   static bool file_read_all_bytes(const std::string& file_path,
                                   std::vector<unsigned char>& file_bytes);
   static bool file_read_all_text(const std::string& file_path,
                                  std::string& file_contents);
   static bool file_copy(const std::string& from_file, const std::string& to_file);
   static bool file_set_permissions(const std::string& file_path,
                                    int user_perms,
                                    int group_perms,
                                    int world_perms);
   static std::vector<std::string> file_read_lines(const std::string& file_path);
   static bool directory_delete_directory(const std::string& dir_path);
   static std::string md5_for_file(const std::string& ini_file_name,
                                   const std::string& path_to_file);
   static bool file_get_mtime(const std::string& file_path, double& mtime);
   static bool execute_program(const std::string& program_path,
                               const std::vector<std::string>& program_args,
                               int& exit_code,
                               std::string& std_out,
                               std::string& std_err);
   static bool launch_program(const std::string& program_path,
                              const std::vector<std::string>& program_args,
                              int& child_process_pid);
   static std::string get_platform_identifier();
   static bool get_platform_config_value(const std::string& ini_file_name,
                                         const std::string& key,
                                         std::string& config_value,
                                         bool strip_quotes=true);
   static bool get_platform_config_values(const std::string& ini_file_name,
                                          chaudiere::KeyValuePairs& kvp);

};

//}

#endif
