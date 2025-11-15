// jukebox.cpp

#include <stdio.h>
#include <signal.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>
#include <random>
#include <algorithm>

#include "jukebox.h"
#include "jukebox_db.h"
#include "file_metadata.h"
#include "song_metadata.h"
#include "song_downloader.h"
#include "jb_utils.h"
#include "utils.h"
#include "IniReader.h"
#include "KeyValuePairs.h"
#include "OSUtils.h"
#include "StrUtils.h"
#include "PthreadsThread.h"
#include "nlohmann/json.hpp"

using namespace std;
using json = nlohmann::json;
using namespace chaudiere;

static Jukebox* g_jukebox_instance = nullptr;

static const string JSON_FILE_EXT = ".json";
static const string ini_file_name = "audio_player.ini";

//*****************************************************************************

void signal_handler(int signum) {
   if (g_jukebox_instance != nullptr) {
      if (signum == SIGUSR1) {
         g_jukebox_instance->toggle_pause_play();
      } else if (signum == SIGUSR2) {
         g_jukebox_instance->advance_to_next_song();
      } else if (signum == SIGINT) {
         g_jukebox_instance->prepare_for_termination();
      } else if (signum == SIGWINCH) {
         g_jukebox_instance->display_info();
      }
   }
}

//*****************************************************************************

void install_signal_handlers() {
   signal(SIGUSR1, signal_handler);
   signal(SIGUSR2, signal_handler);
   signal(SIGINT, signal_handler);
   signal(SIGWINCH, signal_handler);
}

//*****************************************************************************

Jukebox::Jukebox(const JukeboxOptions& jb_options,
                 StorageSystem& storage_sys,
                 bool debugging) :
   m_jukebox_options(jb_options),
   m_storage_system(storage_sys),
   m_debug_print(debugging),
   m_download_extension(".download"),
   m_metadata_db_file("jukebox_db.sqlite3"),
   m_metadata_container("music-metadata"),
   m_playlist_container("cj-playlists"),
   m_album_container("cj-albums"),
   m_album_art_container("album-art"),
   m_number_songs(0),
   m_song_index(-1),
   m_audio_player_process(-1),
   m_cumulative_download_bytes(0),
   m_cumulative_download_time(0.0),
   m_exit_requested(false),
   m_is_paused(false),
   m_song_start_time(0.0),
   m_song_seconds_offset(0),
   m_player_active(false),
   m_downloader_ready_to_delete(false),
   m_num_successive_play_failures(0),
   m_song_play_is_resume(false),
   m_is_repeat_mode(false)
{
   g_jukebox_instance = this;

   m_current_dir = OSUtils::getCurrentDirectory();
   m_song_import_dir = OSUtils::pathJoin(m_current_dir, "song-import");
   m_playlist_import_dir = OSUtils::pathJoin(m_current_dir, "playlist-import");
   m_song_play_dir = OSUtils::pathJoin(m_current_dir, "song-play");
   m_album_art_import_dir = OSUtils::pathJoin(m_current_dir,
                                                       "album-art-import");

   if (jb_options.debug_mode) {
      m_debug_print = true;
   }

   if (m_debug_print) {
      printf("current_dir = %s\n", m_current_dir.c_str());
      printf("song_import_dir = %s\n", m_song_import_dir.c_str());
      printf("song_play_dir = %s\n", m_song_play_dir.c_str());
   }
}

//*****************************************************************************

Jukebox::~Jukebox() {
   g_jukebox_instance = nullptr;

   exit();
}

//*****************************************************************************

bool Jukebox::enter() {
   if (m_debug_print) {
      printf("Jukebox.enter\n");
   }

   // look for stored metadata in the storage system
   if (m_storage_system.has_container(m_metadata_container) &&
       !m_jukebox_options.suppress_metadata_download) {

      // metadata container exists, retrieve container listing
      vector<string> container_contents =
         m_storage_system.list_container_contents(m_metadata_container);

      bool metadata_db_file_found = false;

      for (const auto& object_name : container_contents) {
         if (object_name == m_metadata_db_file) {
            metadata_db_file_found = true;
            break;
         }
      }

      // does our metadata DB file exist in the metadata container?
      if (metadata_db_file_found) {
         //printf("metadata DB file exists in container, attempting to download\n");

         // download it
         string metadata_db_file_path = get_metadata_db_file_path();
         string download_file = metadata_db_file_path + ".download";

         //printf("downloading metadata DB to %s\n", download_file.c_str());
         if (m_storage_system.get_object(m_metadata_container,
                                         m_metadata_db_file,
                                         download_file) > 0) {
            // have an existing metadata DB file?
            if (Utils::path_exists(metadata_db_file_path)) {
               if (m_debug_print) {
                  printf("deleting existing metadata DB file\n");
               }
               OSUtils::deleteFile(metadata_db_file_path);
            }
            // rename downloaded file
            if (m_debug_print) {
               printf("renaming %s to %s\n",
                      download_file.c_str(),
                      metadata_db_file_path.c_str());
            }
            Utils::rename_file(download_file, metadata_db_file_path);
         } else {
            if (m_debug_print) {
               printf("error: unable to retrieve metadata DB file\n");
            }
         }
      } else {
         if (m_debug_print) {
            printf("no metadata DB file in metadata container\n");
         }
      }
   } else {
      if (m_debug_print) {
         printf("no metadata container in storage system\n");
      }
   }

   m_jukebox_db.reset(new JukeboxDB(get_metadata_db_file_path()));
   if (!m_jukebox_db->open()) {
      printf("unable to connect to database\n");
      return false;
   }

   return true;
}

//*****************************************************************************

void Jukebox::exit() {
   if (m_debug_print) {
      printf("Jukebox.exit\n");
   }

   if (m_jukebox_db) {
      if (m_jukebox_db->is_open()) {
         m_jukebox_db->close();
      }
      m_jukebox_db.reset();
   }
}

//*****************************************************************************

void Jukebox::toggle_pause_play() {
   m_is_paused = !m_is_paused;
   m_num_successive_play_failures = 0;
   if (m_is_paused) {
      printf("paused\n");
      if (m_audio_player_process > 0) {
         // capture current song position (seconds into song)
         kill(m_audio_player_process, SIGTERM);
         m_audio_player_process = -1;
      }
   } else {
      printf("resuming play\n");
      m_song_play_is_resume = true;
   }
}

//*****************************************************************************

void Jukebox::advance_to_next_song() {
   printf("advancing to next song\n");
   if (m_audio_player_process > 0) {
      kill(m_audio_player_process, SIGTERM);
      m_audio_player_process = -1;
      m_num_successive_play_failures = 0;
      m_song_play_is_resume = false;
   }
}

//*****************************************************************************

string Jukebox::get_metadata_db_file_path() {
   return OSUtils::pathJoin(m_current_dir, m_metadata_db_file);
}

//*****************************************************************************

vector<string> Jukebox::components_from_file_name(const string& file_name) {
   if (file_name.length() == 0) {
      return vector<string>();
   }

   size_t pos_extension = file_name.find('.');
   string base_file_name = "";
   if (pos_extension != string::npos) {
      base_file_name = file_name.substr(0, pos_extension);
   } else {
      base_file_name = file_name;
   }

   vector<string> tokens = StrUtils::split(base_file_name, "--");
   if (tokens.size() == 3) {
      vector<string> ret_vet(3);
      ret_vet.push_back(JBUtils::unencode_value(tokens[0]));
      ret_vet.push_back(JBUtils::unencode_value(tokens[1]));
      ret_vet.push_back(JBUtils::unencode_value(tokens[2]));
      return ret_vet;
   } else {
      return vector<string>();
   }
}

//*****************************************************************************

string Jukebox::artist_from_file_name(const string& file_name) {
   if (!file_name.empty()) {
      vector<string> components = components_from_file_name(file_name);
      if (components.size() == 3) {
         return components[0];
      }
   }
   return string("");
}

//*****************************************************************************

string Jukebox::album_from_file_name(const string& file_name) {
   if (!file_name.empty()) {
      vector<string> components = components_from_file_name(file_name);
      if (components.size() == 3) {
         return components[1];
      }
   }
   return string("");
}

//*****************************************************************************

string Jukebox::song_from_file_name(const string& file_name) {
   if (!file_name.empty()) {
      vector<string> components = components_from_file_name(file_name);
      if (components.size() == 3) {
         return components[2];
      }
   }
   return string("");
}

//*****************************************************************************

void Jukebox::string_to_vector(const string& s, vector<unsigned char>& v) {
   std::copy(s.begin(), s.end(), std::back_inserter(v));
}

//*****************************************************************************

void Jukebox::vector_to_string(const vector<unsigned char>& v, string& s) {
   // ensure that string has required capacity
   s.reserve(v.size());

   // copy each character
   for (const auto& u_char : v) {
      char s_char = (char) u_char;
      // append character to string
      s.push_back(s_char);
   }
}

//*****************************************************************************

bool Jukebox::store_song_metadata(const SongMetadata& fs_song) {
   SongMetadata db_song;
   if (m_jukebox_db->retrieve_song(fs_song.fm.file_uid, db_song)) {
      if (fs_song != db_song) {
         return m_jukebox_db->update_song(fs_song);
      } else {
         return true;  // no insert or update needed (already up-to-date)
      }
   } else {
      // song is not in the database, insert it
      return m_jukebox_db->insert_song(fs_song);
   }
}

//*****************************************************************************

void Jukebox::get_encryptor() {
   //FUTURE: encryption (get_encryptor)
   // key_block_size = 16  // AES-128
   // key_block_size = 24  // AES-192
   //int key_block_size = 32;  // AES-256
   //return AESBlockEncryption(key_block_size,
   //                          jukebox_options.encryption_key,
   //                          jukebox_options.encryption_iv);
}

//*****************************************************************************

string Jukebox::get_container_suffix() {
   string suffix = "";
   if (m_jukebox_options.use_encryption && m_jukebox_options.use_compression) {
      suffix += "-ez";
   } else if (m_jukebox_options.use_encryption) {
      suffix += "-e";
   } else if (m_jukebox_options.use_compression) {
      suffix += "-z";
   }
   return suffix;
}

//*****************************************************************************

string Jukebox::object_file_suffix() {
   string suffix = "";
   if (m_jukebox_options.use_encryption && m_jukebox_options.use_compression) {
      suffix = ".egz";
   } else if (m_jukebox_options.use_encryption) {
      suffix = ".e";
   } else if (m_jukebox_options.use_compression) {
      suffix = ".gz";
   }
   return suffix;
}

//*****************************************************************************

string Jukebox::container_for_song(const string& song_uid) {
   if (song_uid.length() == 0) {
      return string("");
   }
   string container_suffix = "-artist-songs" + get_container_suffix();

   string artist = artist_from_file_name(song_uid);
   string artist_letter = "";
   if (StrUtils::startsWith(artist, "A ")) {
      artist_letter = artist.substr(2, 1);
   } else if (StrUtils::startsWith(artist, "The ")) {
      artist_letter = artist.substr(4, 1);
   } else {
      artist_letter = artist.substr(0, 1);
   }

   string container_name = artist_letter;
   StrUtils::toLowerCase(container_name);
   container_name += container_suffix;

   return container_name;
}

//*****************************************************************************

void Jukebox::import_songs() {
   if (m_jukebox_db && m_jukebox_db->is_open()) {
      vector<string> dir_listing =
         OSUtils::listFilesInDirectory(m_song_import_dir);
      float num_entries = (float) dir_listing.size();
      double progressbar_chars = 0.0;
      int progressbar_width = 40;
      int progress_chars_per_iteration = (int) (progressbar_width / num_entries);
      char progressbar_char = '#';
      int bar_chars = 0;

      if (!m_debug_print) {
         // setup progressbar
         string bar = StrUtils::makeStringOfChar('*', progressbar_width);
         string bar_text = "[" + bar + "]";
         Utils::sys_stdout_write(bar_text);
         Utils::sys_stdout_flush();
         bar = StrUtils::makeStringOfChar('\b', progressbar_width + 1);
         Utils::sys_stdout_write(bar);  // return to start of line, after '['
      }

      //FUTURE: encryption support (import_songs)
      //if (jukebox_options != nullptr && jukebox_options.use_encryption) {
      //   encryption = get_encryptor();
      //} else {
      //   encryption = nullptr;
      //}

      double cumulative_upload_time = 0.0;
      int cumulative_upload_bytes = 0;
      int file_import_count = 0;

      for (const auto& listing_entry : dir_listing) {
         string full_path = OSUtils::pathJoin(m_song_import_dir,
                                              listing_entry);
         // ignore it if it's not a file
         if (Utils::path_isfile(full_path)) {
            string file_name = listing_entry;
            vector<string> path_elems = Utils::path_splitext(full_path);
            const string& extension = path_elems[1];
            if (!extension.empty()) {
               long file_size = Utils::get_file_size(full_path);
               string artist = artist_from_file_name(file_name);
               string album = album_from_file_name(file_name);
               string song = song_from_file_name(file_name);
               if (file_size > 0 &&
                   !artist.empty() &&
                   !album.empty() &&
                   !song.empty()) {

                  string object_name = file_name + object_file_suffix();
                  SongMetadata fs_song;
                  fs_song.fm.file_uid = object_name;
                  fs_song.album_uid = "";
                  fs_song.fm.origin_file_size = (int) file_size;
                  fs_song.fm.file_time =
                     Utils::datetime_datetime_fromtimestamp(Utils::path_getmtime(full_path));
                  fs_song.artist_name = artist;
                  fs_song.song_name = song;
                  fs_song.fm.md5_hash = Utils::md5_for_file(ini_file_name, full_path);
                  fs_song.fm.compressed = m_jukebox_options.use_compression ? 1 : 0;
                  fs_song.fm.encrypted = m_jukebox_options.use_encryption ? 1 : 0;
                  fs_song.fm.object_name = object_name;
                  fs_song.fm.pad_char_count = 0;

                  fs_song.fm.container_name = container_for_song(file_name);

                  // read file contents
                  bool file_read = false;
                  vector<unsigned char> file_contents;

                  if (Utils::file_read_all_bytes(full_path, file_contents)) {
                     file_read = true;
                  } else {
                     printf("error: unable to read file %s\n", full_path.c_str());
                  }

                  if (file_read && file_contents.size() > 0) {
                     if (file_contents.size() > 0) {
                        // for general purposes, it might be useful or helpful to have
                        // a minimum size for compressing
                        if (m_jukebox_options.use_compression) {
                           if (m_debug_print) {
                              printf("compressing file\n");
                           }

                           //FUTURE: compression (import_songs)
                           //file_bytes = bytes(file_contents, 'utf-8');
                           //file_contents = zlib.compress(file_bytes, 9);
                        }

                        if (m_jukebox_options.use_encryption) {
                           if (m_debug_print) {
                              printf("encrypting file\n");
                           }

                           //FUTURE: encryption (import_songs)

                           // the length of the data to encrypt must be a multiple of 16
                           //num_extra_chars = file_contents.Length % 16;
                           //if (num_extra_chars > 0) {
                           //   if (debug_print) {
                           //      printf("padding file for encryption\n");
                           //   }
                           //   num_pad_chars = 16 - num_extra_chars;
                           //   file_contents += "".ljust(num_pad_chars, ' ');
                           //   fs_song.fm.pad_char_count = num_pad_chars;
                           //}

                           //file_contents = encryption.encrypt(file_contents);
                        }
                     }

                     // now that we have the data that will be stored, set the file size for
                     // what's being stored
                     fs_song.fm.stored_file_size = file_contents.size();
                     double start_upload_time = Utils::time_time();

                     // store song file to storage system
                     if (m_storage_system.put_object(fs_song.fm.container_name,
                                                     fs_song.fm.object_name,
                                                     file_contents,
                                                     nullptr)) {
                        double end_upload_time = Utils::time_time();
                        double upload_elapsed_time = end_upload_time - start_upload_time;
                        cumulative_upload_time += upload_elapsed_time;
                        cumulative_upload_bytes += file_contents.size();

                        // store song metadata in local database
                        if (!store_song_metadata(fs_song)) {
                           // we stored the song to the storage system, but were unable to store
                           // the metadata in the local database. we need to delete the song
                           // from the storage system since we won't have any way to access it
                           // since we can't store the song metadata locally.
                           printf("unable to store metadata, deleting obj %s\n",
                                  fs_song.fm.object_name.c_str());

                           m_storage_system.delete_object(fs_song.fm.container_name,
                                                          fs_song.fm.object_name);
                        } else {
                           file_import_count += 1;
                        }
                     } else {
                        printf("error: unable to upload %s to %s\n",
                               fs_song.fm.object_name.c_str(),
                               fs_song.fm.container_name.c_str());
                     }
                  }
               }
            }

            if (!m_debug_print) {
               progressbar_chars += progress_chars_per_iteration;
               if (progressbar_chars > bar_chars) {
                  int num_new_chars = (int) (progressbar_chars - bar_chars);
                  if (num_new_chars > 0) {
                     // update progress bar
                     for (int j = 0; j < num_new_chars; j++) {
                        printf("%c", progressbar_char);
                     }
                     Utils::sys_stdout_flush();
                     bar_chars += num_new_chars;
                  }
               }
            }
         }
      }

      if (!m_debug_print) {
         // if we haven't filled up the progress bar, fill it now
         if (bar_chars < progressbar_width) {
            int num_new_chars = progressbar_width - bar_chars;
            for (int j = 0; j < num_new_chars; j++) {
               printf("%c", progressbar_char);
            }
            Utils::sys_stdout_flush();
         }
         printf("\n");
      }

      if (file_import_count > 0) {
         upload_metadata_db();
      } else {
         printf("DEBUG: file_import_count == 0, not uploading metadata DB\n");
      }

      printf("%d song files imported\n", file_import_count);

      if (cumulative_upload_time > 0) {
         double cumulative_upload_kb = cumulative_upload_bytes / 1000.0;
         int avg = (int) (cumulative_upload_kb / cumulative_upload_time);
         printf("average upload throughput = %d KB/sec\n", avg);
      }
   }
}

//*****************************************************************************

string Jukebox::song_path_in_playlist(const SongMetadata& song) {
   return OSUtils::pathJoin(m_song_play_dir, song.fm.file_uid);
}

//*****************************************************************************

bool Jukebox::check_file_integrity(const SongMetadata& song) {
   bool file_integrity_passed = true;

   if (m_jukebox_options.check_data_integrity) {
      string file_path = song_path_in_playlist(song);
      if (Utils::file_exists(file_path)) {
         if (m_debug_print) {
            printf("checking integrity for %s\n", song.fm.file_uid.c_str());
         }

         string playlist_md5 = Utils::md5_for_file(ini_file_name, file_path);
         if (playlist_md5 == song.fm.md5_hash) {
            if (m_debug_print) {
               printf("integrity check SUCCESS\n");
            }
            file_integrity_passed = true;
         } else {
            printf("file integrity check failed: %s\n",
                   song.fm.file_uid.c_str());
            file_integrity_passed = false;
         }
      } else {
         // file doesn't exist
         printf("error: check_file_integrity - file doesn't exist\n");
         file_integrity_passed = false;
      }
   } else {
      if (m_debug_print) {
         printf("file integrity bypassed, no jukebox options or check integrity not turned on\n");
      }
   }

   return file_integrity_passed;
}

//*****************************************************************************

void Jukebox::batch_download_start() {
   m_cumulative_download_bytes = 0;
   m_cumulative_download_time = 0.0;
}

//*****************************************************************************

void Jukebox::batch_download_complete() {
   if (!m_exit_requested) {
      if (m_cumulative_download_time > 0) {
         double cumulative_download_kb = m_cumulative_download_bytes / 1000.0;
         int avg = (int) (cumulative_download_kb / m_cumulative_download_time);
         printf("average download throughput = %d KB/sec\n", avg);
      }

      m_cumulative_download_bytes = 0;
      m_cumulative_download_time = 0.0;
   }
}

//*****************************************************************************

void Jukebox::notifyRunComplete(Runnable* runnable) {
   if (runnable == m_downloader.get()) {
      m_downloader_ready_to_delete = true;
   }
}

//*****************************************************************************

bool Jukebox::download_song(const SongMetadata& song) {
   if (m_debug_print) {
      printf("download_song called for '%s'\n", song.fm.file_uid.c_str());
   }

   if (m_exit_requested) {
      printf("download_song returning false because exit_requested\n");
      return false;
   }

   //printf("attempting to download song '%s'\n", song.fm.file_uid.c_str());

   string file_path = song_path_in_playlist(song);
   double download_start_time = Utils::time_time();
   unsigned long song_bytes_retrieved =
      m_storage_system.retrieve_file(song.fm, m_song_play_dir);
   if (m_debug_print) {
      printf("song_bytes_retrieved = %ld\n", song_bytes_retrieved);
   }

   if (m_exit_requested) {
      printf("download_song returning false because exit_requested\n");
      return false;
   }

   //if (m_debug_print) {
   //   printf("bytes retrieved: %ld\n", song_bytes_retrieved);
   //}

   if (song_bytes_retrieved > 0) {
      double download_end_time = Utils::time_time();
      double download_elapsed_time = download_end_time - download_start_time;
      m_cumulative_download_time += download_elapsed_time;
      m_cumulative_download_bytes += song_bytes_retrieved;

      // are we checking data integrity?
      // if so, verify that the storage system retrieved the same length that
      // has been stored
      if (m_jukebox_options.check_data_integrity) {
         //printf("checking data integrity\n");
         if (m_debug_print) {
            printf("verifying data integrity\n");
         }

         if (song_bytes_retrieved != song.fm.stored_file_size) {
            printf("error: data integrity check failed for %s\n",
                   file_path.c_str());
            return false;
         }
      }

      // is it encrypted? if so, unencrypt it
      //int encrypted = song.fm.encrypted;
      //int compressed = song.fm.compressed;

      //FUTURE: encryption and compression (download_song)
      //if (encrypted == 1 || compressed == 1) {
      //     try:
      //         with open(file_path, 'rb') as content_file:
      //             file_contents = content_file.read()
      //     except IOError:
      //         printf("error: unable to read file %s\n", file_path.c_str())
      //         return false

      //     if (encrypted) {
      //        encryption = get_encryptor()
      //        file_contents = encryption.decrypt(file_contents)
      //     }
      //     if (compressed) {
      //        file_contents = zlib.decompress(file_contents)
      //     }

           // re-write out the uncompressed, unencrypted file contents
      //     try:
      //         with open(file_path, 'wb') as content_file:
      //             content_file.write(file_contents)
      //     except IOError:
      //         print("error: unable to write unencrypted/uncompressed file '%s'\n", file_path.c_str())
      //         return false
      //}

      if (check_file_integrity(song)) {
         if (m_debug_print) {
            printf("check_file_integrity returned true\n");
         }
         return true;
      } else {
         // we retrieved the file, but it failed our integrity check
         // if file exists, remove it
         printf("integrity check failed, deleting file\n");
         if (Utils::file_exists(file_path)) {
            OSUtils::deleteFile(file_path);
         }
      }
   }

   return false;
}

//*****************************************************************************

void Jukebox::play_song(const SongMetadata& song) {
   if (m_player_active) {
      return;
   }

   string song_file_path = song_path_in_playlist(song);

   if (Utils::path_exists(song_file_path)) {
      printf("playing %s\n", song.fm.file_uid.c_str());

      if (!m_audio_player_exe_file_name.empty()) {
         bool did_resume = false;
         string command_args;
         if (m_song_play_is_resume) {
            string placeholder = "%%START_SONG_TIME_OFFSET%%";
            string::size_type pos_placeholder =
               m_audio_player_resume_args.find(placeholder);
            if (pos_placeholder != string::npos) {
               command_args = m_audio_player_resume_args;
               string song_start_time;
               int minutes = m_song_seconds_offset / 60;
               if (minutes > 0) {
                  song_start_time = StrUtils::toString(minutes);
                  song_start_time += ":";
                  int remaining_seconds = m_song_seconds_offset % 60;
                  string seconds_text =
                     StrUtils::toString(remaining_seconds);
                  if (seconds_text.length() == 1) {
                     seconds_text = string("0") + seconds_text;
                  }
                  song_start_time += seconds_text;
               } else {
                  song_start_time =
                     StrUtils::toString(m_song_seconds_offset);
               }
               //printf("resuming at '%s'\n", song_start_time.c_str());
               StrUtils::replaceAll(command_args,
                                    "%%START_SONG_TIME_OFFSET%%",
                                    song_start_time);
               StrUtils::replaceAll(command_args,
                                    "%%AUDIO_FILE_PATH%%",
                                    song_file_path);
               did_resume = true;
               //printf("command_args: '%s'\n", command_args.c_str());
            }
         }

         if (!did_resume) {
            command_args = m_audio_player_command_args;
            StrUtils::replaceAll(command_args,
                                 "%%AUDIO_FILE_PATH%%",
                                 song_file_path);
         }

         vector<string> vec_args = StrUtils::split(command_args, " ");
         int child_process_id = 0;
         pid_t pid;
         int exit_code = -1;

         bool started_audio_player = Utils::launch_program(m_audio_player_exe_file_name,
                                                           vec_args,
                                                           child_process_id);
         if (started_audio_player) {
            pid = child_process_id;
            m_player_active = true;
            m_song_start_time = Utils::time_time();
            int status = 0;
            int options = 0;
            m_audio_player_process = pid;
            pid_t rc_pid = waitpid(pid, &status, options);
            if (rc_pid == pid) {
               if (WIFEXITED(status)) {
                  exit_code = WEXITSTATUS(status);
                  double song_end_time = Utils::time_time();
                  double song_play_time = song_end_time - m_song_start_time;
                  m_song_seconds_offset += floor(song_play_time);
                  //printf("song_start_time = %f\n", song_start_time);
                  //printf("song_end_time = %f\n", song_end_time);
                  //printf("song_play_time = %f\n", song_play_time);
                  //printf("DEBUG: song_seconds_offset = %d\n", song_seconds_offset);
                  m_player_active = false;
                  if (exit_code == 0) {
                     m_num_successive_play_failures = 0;
                  }
                  m_song_play_is_resume = false;
               } else {
                  printf("waitpid returned, but player not exited\n");
               }
            } else {
               printf("waitpid return value (other than player pid) = %d\n",
                      rc_pid);
               printf("errno = %d\n", errno);
            }
            m_audio_player_process = -1;
            m_player_active = false;
         } else {
            printf("error: unable to start audio player\n");
            ::exit(1);
         }

         // audio player failed or is not present?
         if (!started_audio_player || exit_code != 0) {
            ++m_num_successive_play_failures;
            if (m_num_successive_play_failures >= 3) {
               // we've had at least 3 successive play failures.
               // obviously something is not right with config.
               // just print a message and exit.
               printf("error: audio player appears to be misconfigured. exiting\n");
               ::exit(1);
            }
         }
      } else {
         // we don't know about an audio player, so there's nothing
         // left to do
         printf("error: no audio player configured. exiting.\n");
         ::exit(1);
      }

      if (!m_is_paused) {
         // delete the song file from the play list directory
         OSUtils::deleteFile(song_file_path);
      }
   } else {
      printf("file not found: %s\n", song.fm.file_uid.c_str());
      Utils::file_append_all_text("404.txt", song.fm.file_uid);
   }
}

//*****************************************************************************

void Jukebox::download_songs() {
   // scan the play list directory to see if we need to download more songs
   vector<string> dir_listing =
      OSUtils::listFilesInDirectory(m_song_play_dir);
   unsigned int song_file_count = 0;

   for (const auto& listing_entry : dir_listing) {
      string full_path =
         OSUtils::pathJoin(m_song_play_dir, listing_entry);
      if (Utils::path_isfile(full_path)) {
         vector<string> path_elems = Utils::path_splitext(full_path);
         const string& extension = path_elems[1];
         if (!extension.empty() && extension != m_download_extension) {
            song_file_count += 1;
         }
      }
   }

   unsigned int file_cache_count = m_jukebox_options.file_cache_count;

   if (song_file_count < file_cache_count) {
      vector<SongMetadata> dl_songs;
      // start looking at the next song in the list
      int check_index = m_song_index + 1;

      for (int j = 0; j < m_number_songs; j++) {
         if (check_index >= m_number_songs) {
            check_index = 0;
         }
         if (check_index != m_song_index) {
            const SongMetadata& si = m_song_list[check_index];
            string file_path = song_path_in_playlist(si);
            if (!Utils::file_exists(file_path)) {
               dl_songs.push_back(si);
               if (dl_songs.size() >= file_cache_count) {
                  break;
               }
            }
         }
         check_index++;
      }

      if (dl_songs.size() > 0) {
         if (!m_downloader && !m_download_thread) {
            if (m_debug_print) {
               printf("creating SongDownloader and download thread\n");
            }
            m_downloader.reset(new SongDownloader(*this, dl_songs));
            m_download_thread.reset(new PthreadsThread(m_downloader.get()));
            m_downloader->setCompletionObserver(this);
            m_download_thread->start();
         } else {
            if (m_debug_print) {
               printf("Not downloading more songs b/c downloader != nullptr or download_thread != nullptr\n");
            }
         }
      }
   }
}

//*****************************************************************************

void Jukebox::downloader_cleanup() {
   if (m_downloader_ready_to_delete) {
      if (m_downloader && m_download_thread) {
         //printf("deleting downloader and download thread\n");
         m_downloader_ready_to_delete = false;
         m_downloader.reset();         
         m_download_thread.reset();
      } else {
         printf("NOT deleting b/c 1 or both is nullptr\n");
      }
   }
}

//*****************************************************************************

void Jukebox::play_songs(bool shuffle, string artist, string album) {
   if (m_jukebox_db) {
      bool have_songs = false;
      if (!artist.empty() && !album.empty()) {
         vector<SongMetadata> a_song_list;
         vector<string> list_track_objects;
         if (retrieve_album_track_object_list(artist,
                                              album,
                                              list_track_objects)) {
            if (list_track_objects.size() > 0) {
               for (const auto& track_object_name : list_track_objects) {
                  SongMetadata song;
                  if (m_jukebox_db->retrieve_song(track_object_name, song)) {
                     a_song_list.push_back(song);
                  }
               }

               if (a_song_list.size() == list_track_objects.size()) {
                  have_songs = true;
                  m_song_list = a_song_list;
               }
            }
         }
      }

      if (!have_songs) {
         m_song_list = m_jukebox_db->retrieve_album_songs(artist, album);
      }

      play_retrieved_songs(shuffle);
   }
}

//*****************************************************************************

void Jukebox::play_retrieved_songs(bool shuffle) {
   if (!m_song_list.empty()) {
      m_number_songs = m_song_list.size();

      if (m_number_songs == 0) {
         printf("no songs in jukebox\n");
         return;
      }

      // does play list directory exist?
      if (!OSUtils::directoryExists(m_song_play_dir)) {
         if (m_debug_print) {
            printf("song-play directory does not exist, creating it\n");
         }
         OSUtils::createDirectory(m_song_play_dir);
      } else {
         // play list directory exists, delete any files in it
         if (m_debug_print) {
            printf("deleting existing files in song-play directory\n");
         }

         vector<string> list_files =
            OSUtils::listFilesInDirectory(m_song_play_dir);
         for (const auto& theFile : list_files) {
            string file_path =
               OSUtils::pathJoin(m_song_play_dir, theFile);
            if (Utils::path_isfile(file_path)) {
               OSUtils::deleteFile(file_path);
            }
         }
      }

      m_song_index = 0;
      install_signal_handlers();

      string os_identifier = Utils::get_platform_identifier();
      if (os_identifier == "unknown") {
         printf("error: no audio-player specific lookup defined for this OS (unknown)\n");
         return;
      }

      m_audio_player_exe_file_name = "";
      m_audio_player_command_args = "";
      m_audio_player_resume_args = "";

      try {
         IniReader ini_reader(ini_file_name);
         KeyValuePairs kvpAudioPlayer;
         if (!ini_reader.readSection(os_identifier, kvpAudioPlayer)) {
            printf("error: no config section present for '%s'\n",
                   os_identifier.c_str());
            return;
         }

         string key = "audio_player_exe_file_name";
         if (kvpAudioPlayer.hasKey(key)) {
            m_audio_player_exe_file_name = kvpAudioPlayer.getValue(key);
            if (StrUtils::startsWith(m_audio_player_exe_file_name, "\"") &&
                StrUtils::endsWith(m_audio_player_exe_file_name, "\"")) {
               StrUtils::strip(m_audio_player_exe_file_name, '"');
            }
            StrUtils::strip(m_audio_player_exe_file_name);
            if (m_audio_player_exe_file_name.empty()) {
               printf("error: no value given for '%s' within [%s]\n",
                      key.c_str(),
                      os_identifier.c_str());
               return;
            }
         } else {
            printf("error: audio_player.ini missing value for '%s' within [%s]\n",
                   key.c_str(),
                   os_identifier.c_str());
            return;
         }

         key = "audio_player_command_args";
         if (kvpAudioPlayer.hasKey(key)) {
            m_audio_player_command_args = kvpAudioPlayer.getValue(key);
            if (StrUtils::startsWith(m_audio_player_command_args, "\"") &&
                StrUtils::endsWith(m_audio_player_command_args, "\"")) {

               StrUtils::strip(m_audio_player_command_args, '"');
            }
            StrUtils::strip(m_audio_player_command_args);
            if (m_audio_player_command_args.empty()) {
               printf("error: no value given for '%s' within [%s]\n",
                      key.c_str(),
                      os_identifier.c_str());
               return;
            }

            string placeholder = "%%AUDIO_FILE_PATH%%";
            string::size_type pos_placeholder =
               m_audio_player_command_args.find(placeholder);
            if (pos_placeholder == string::npos) {
               printf("error: %s value does not contain placeholder '%s'\n",
                      key.c_str(),
                      placeholder.c_str());
               return;
            }

         } else {
            printf("error: audio_player.ini missing value for '%s' within [%s]\n",
                   key.c_str(),
                   os_identifier.c_str());
            return;
         }

         key = "audio_player_resume_args";
         if (kvpAudioPlayer.hasKey(key)) {
            m_audio_player_resume_args = kvpAudioPlayer.getValue(key);
            if (StrUtils::startsWith(m_audio_player_resume_args, "\"") &&
                StrUtils::endsWith(m_audio_player_resume_args, "\"")) {
               StrUtils::strip(m_audio_player_resume_args, '"');
            }
            StrUtils::strip(m_audio_player_resume_args);
            if (!m_audio_player_resume_args.empty()) {
               string placeholder = "%%START_SONG_TIME_OFFSET%%";
               string::size_type pos_placeholder =
                  m_audio_player_resume_args.find(placeholder);
               if (pos_placeholder == string::npos) {
                  printf("error: %s value does not contain placeholder '%s'\n",
                         key.c_str(),
                         placeholder.c_str());
                  printf("ignoring '%s', using 'audio_player_command_args' for song resume\n",
                         key.c_str());
                  m_audio_player_resume_args = "";
               }
            }
         }

         if (m_audio_player_resume_args.empty()) {
            m_audio_player_resume_args = m_audio_player_command_args;
         }
      } catch (const exception& e) {
         printf("error: unable to read %s - %s\n", ini_file_name.c_str(), e.what());
         return;
      }

      if (m_debug_print) {
         printf("audio_player_exe_file_name = '%s'\n",
                m_audio_player_exe_file_name.c_str());
         printf("audio_player_command_args = '%s'\n",
                m_audio_player_command_args.c_str());
      }

      printf("downloading first song...\n");

      if (shuffle) {
         std::random_device rd;
         std::default_random_engine rng(rd());
         std::shuffle(m_song_list.begin(), m_song_list.end(), rng);
      }

      try
      {
         if (download_song(m_song_list[0])) {
            printf("first song downloaded. starting playing now.\n");

            // write PID to "jukebox.pid"
            int pid_value = Utils::get_pid();
            char pid_text[128];
            memset(pid_text, 0, sizeof(pid_text));
            snprintf(pid_text, 128, "%d\n", pid_value);
            string str_pid_text = pid_text;
            Utils::file_write_all_text("jukebox.pid", str_pid_text);

            while (!m_exit_requested) {
               downloader_cleanup();

               if (!m_is_paused) {
                  if (!m_downloader && !m_download_thread) {
                     if (m_debug_print) {
                        printf("calling download_songs\n");
                     }
                     download_songs();
                     if (!m_player_active) {
                        play_song(m_song_list[m_song_index]);
                     }
                     downloader_cleanup();
                  } else {
                     if (m_debug_print) {
                        printf("have downloader so not downloading\n");
                     }
                  }
               }

               if (!m_is_paused) {
                  m_song_index++;
                  m_song_play_is_resume = false;
                  m_song_seconds_offset = 0;
                  if (m_song_index >= m_number_songs) {
                     if (m_is_repeat_mode) {
                        m_song_index = 0;
                     } else {
                        m_exit_requested = true;
                     }
                  }
               } else {
                  Utils::time_sleep(1);
               }
            }
            OSUtils::deleteFile("jukebox.pid");
         } else {
            printf("error: unable to download songs\n");
            return;
         }
      } catch (exception& e) {
         printf("exception caught: %s\n", e.what());
         printf("\nexiting jukebox\n");
         OSUtils::deleteFile("jukebox.pid");
         m_exit_requested = true;
      }
   }
}

//*****************************************************************************

void Jukebox::show_list_containers() {
   vector<string> containers = m_storage_system.list_account_containers();
   if (!containers.empty()) {
      for (const auto& container_name : containers) {
         printf("%s\n", container_name.c_str());
      }
   } else {
      printf("no containers found\n");
   }
}

//*****************************************************************************

void Jukebox::show_listings() {
   if (m_jukebox_db) {
      m_jukebox_db->show_listings();
   }
}

//*****************************************************************************

void Jukebox::show_artists() {
   if (m_jukebox_db) {
      m_jukebox_db->show_artists();
   }
}

//*****************************************************************************

void Jukebox::show_genres() {
   if (m_jukebox_db) {
      m_jukebox_db->show_genres();
   }
}

//*****************************************************************************

void Jukebox::show_albums() {
   if (m_jukebox_db) {
      m_jukebox_db->show_albums();
   }
}

//*****************************************************************************

ReadFileResults Jukebox::read_file_contents(const string& file_path,
                                            bool allow_encryption) {
   bool file_read = false;
   vector<unsigned char> file_contents;
   int pad_chars = 0;

   if (Utils::file_read_all_bytes(file_path, file_contents)) {
      file_read = true;
   } else {
      printf("error: unable to read file %s\n", file_path.c_str());
   }

   if (file_read && file_contents.size() > 0) {
      if (file_contents.size() > 0) {
         // for general purposes, it might be useful or helpful to have
         // a minimum size for compressing
         if (m_jukebox_options.use_compression) {
            //FUTURE: compression (read_file_contents)
            /*
            if (debug_print) {
               printf("compressing file\n");
            }

            file_bytes = bytes(file_contents, 'utf-8');
            file_contents = zlib.compress(file_bytes, 9);
            */
         }

         if (allow_encryption && m_jukebox_options.use_encryption) {
            //FUTURE: encryption (read_file_contents)
            /*
            if (debug_print) {
               printf("encrypting file\n");
            }

            // the length of the data to encrypt must be a multiple of 16
            int num_extra_chars = file_contents.Length % 16;
            if (num_extra_chars > 0) {
                if (debug_print) {
                   printf("padding file for encryption\n");
                }
                pad_chars = 16 - num_extra_chars;
                file_contents += "".ljust(pad_chars, ' ');
            }

            file_contents = encryption.encrypt(file_contents);
            */
         }
      }
   }

   return ReadFileResults(file_read, file_contents, pad_chars);
}

//*****************************************************************************

bool Jukebox::upload_metadata_db() {
   bool metadata_db_upload = false;

   if (m_debug_print) {
      printf("uploading metadata db file to storage system\n");
   }

   m_jukebox_db->close();
   m_jukebox_db.reset();

   // upload metadata DB file
   vector<unsigned char> db_file_contents;
   if (Utils::file_read_all_bytes(get_metadata_db_file_path(),
                                  db_file_contents)) {
      printf("uploading metadata DB to storage system\n");
      metadata_db_upload = m_storage_system.put_object(m_metadata_container,
                                                       m_metadata_db_file,
                                                       db_file_contents,
                                                       nullptr);

      if (m_debug_print) {
         if (metadata_db_upload) {
            printf("metadata db file uploaded\n");
         } else {
            printf("unable to upload metadata db file\n");
         }
      }
   } else {
      printf("file read of metadata DB file failed\n");
   }

   return metadata_db_upload;
}

//*****************************************************************************

void Jukebox::import_playlists() {
   if (m_jukebox_db && m_jukebox_db->is_open()) {
      int file_import_count = 0;
      vector<string> dir_listing =
         OSUtils::listFilesInDirectory(m_playlist_import_dir);
      if (dir_listing.size() == 0) {
         printf("no playlists found\n");
         return;
      }

      for (const auto& listing_entry : dir_listing) {
         printf("'%s'\n", listing_entry.c_str());
         string full_path =
            OSUtils::pathJoin(m_playlist_import_dir, listing_entry);
         // ignore it if it's not a file
         if (Utils::file_exists(full_path)) {
            string object_name = listing_entry;
            string file_contents;
            if (Utils::file_read_all_text(full_path, file_contents)) {
               vector<unsigned char> v_file_contents;
               string_to_vector(file_contents, v_file_contents);
               if (m_storage_system.put_object(m_playlist_container,
                                               object_name,
                                               v_file_contents,
                                               nullptr)) {
                  file_import_count += 1;
               } else {
                  printf("error: unable to store playlist '%s' in '%s'\n",
                         object_name.c_str(), m_playlist_container.c_str());
               }
            } else {
               printf("error: unable to read file '%s'\n", full_path.c_str());
            }
         } else {
            printf("item is not a file: '%s'\n", full_path.c_str());
         }
      }

      if (file_import_count > 0) {
         printf("%d playlists imported\n", file_import_count);
      } else {
         printf("no files imported\n");
      }
   }
}

//*****************************************************************************

void Jukebox::show_playlists() {
   vector<string> container_contents =
      m_storage_system.list_container_contents(m_playlist_container);

   for (const auto& object_name : container_contents) {
      printf("%s\n", object_name.c_str());
   }
}

//*****************************************************************************

bool Jukebox::get_playlist_songs(const string& playlist_name,
                                 vector<SongMetadata>& list_songs) {
   bool success = false;

   string playlist_file = JBUtils::encode_value(playlist_name);
   if (!StrUtils::endsWith(playlist_file, JSON_FILE_EXT)) {
      playlist_file += JSON_FILE_EXT;
   }

   // retrieve the playlist file from storage
   string local_file_path =
      OSUtils::pathJoin(OSUtils::getCurrentDirectory(),
                        playlist_file);
   if (m_storage_system.get_object(m_playlist_container,
                                   playlist_file,
                                   local_file_path) > 0) {
      string file_contents;
      if (Utils::file_read_all_text(local_file_path, file_contents)) {
         json pl_json = json::parse(file_contents);
         // print the list of songs
         if (pl_json.contains("songs")) {
            json songs = pl_json["songs"];
            int songs_added = 0;
            vector<string> file_extensions(3);
            file_extensions.push_back(".flac");
            file_extensions.push_back(".m4a");
            file_extensions.push_back(".mp3");

            for (auto it = songs.begin(); it != songs.end(); it++) {
               json song_dict = it.value();
               if (song_dict.contains("artist") &&
                   song_dict.contains("album") &&
                   song_dict.contains("song")) {

                  string artist = song_dict["artist"];
                  string album = song_dict["album"];
                  string song_name = song_dict["song"];

                  if (!artist.empty() &&
                      !album.empty() &&
                      !song_name.empty()) {
                     string encoded_song =
                        JBUtils::encode_artist_album_song(artist,
                                                          album,
                                                          song_name);
                     encoded_song = JBUtils::remove_punctuation(encoded_song);

                     bool song_found = false;

                     for (const auto& fileExtension : file_extensions) {
                        string song_uid = encoded_song + fileExtension;
                        SongMetadata song;
                        if (m_jukebox_db->retrieve_song(song_uid, song)) {
                           list_songs.push_back(song);
                           songs_added++;
                           song_found = true;
                           break;
                        }
                     }

                     if (!song_found) {
                        printf("error: unable to retrieve metadata for '%s'\n",
                               encoded_song.c_str());
                     }
                  }
               }
            }
            success = (songs_added > 0);
         } else {
            printf("error: playlist json does not contain 'songs' element\n");
         }
      } else {
         printf("error: unable to read file '%s'\n", local_file_path.c_str());
      }
   } else {
      printf("error: playlist not found '%s'\n", playlist_file.c_str());
   }
   return success;
}

//*****************************************************************************

void Jukebox::show_album(const string& artist, const string& album) {
   string json_file_name = JBUtils::encode_artist_album(artist, album) +
                           JSON_FILE_EXT;
   string local_json_file =
      OSUtils::pathJoin(m_song_play_dir, json_file_name);
   if (m_storage_system.get_object(m_album_container,
                                   json_file_name,
                                   local_json_file) > 0) {
      string album_json_contents;
      if (Utils::file_read_all_text(local_json_file, album_json_contents)) {
         if (!album_json_contents.empty()) {
            json album_json = json::parse(album_json_contents);
            if (album_json.contains("tracks")) {
               auto& track_list = album_json["tracks"];

               printf("Album: %s, Artist: %s, Tracks:\n",
                      album.c_str(),
                      artist.c_str());
               int i = 0;
               for (auto& track : track_list) {
                  i++;
                  const string& track_name = track["title"].get<std::string>();
                  if (!track_name.empty()) {
                     printf("%d  %s\n", i, track_name.c_str());
                  }
               }
            }
         } else {
            printf("error: album json file is empty %s\n",
                   local_json_file.c_str());
         }
      } else {
         printf("error: unable to read album json file %s\n",
                local_json_file.c_str());
      }
   } else {
      printf("error: unable to retrieve album json (%s) from storage system (%s)\n",
             json_file_name.c_str(),
             m_album_container.c_str());
   }
}

//*****************************************************************************

void Jukebox::show_playlist(const string& playlist_name) {
   vector<SongMetadata> list_songs;
   if (get_playlist_songs(playlist_name, list_songs)) {
      for (const auto& song : list_songs) {
         printf("%s : %s\n", song.song_name.c_str(), song.artist_name.c_str());
      }
   }
}

//*****************************************************************************

void Jukebox::play_playlist(const string& playlist_name) {
   vector<SongMetadata> list_songs;
   if (get_playlist_songs(playlist_name, list_songs)) {
      m_song_list = list_songs;
      play_retrieved_songs(false);
   } else {
      printf("error: unable to retrieve playlist songs\n");
   }
}

//*****************************************************************************

bool Jukebox::delete_song(const string& song_uid, bool upload_metadata) {
   bool is_deleted = false;
   if (!song_uid.empty()) {
      bool db_deleted = m_jukebox_db->delete_song(song_uid);
      string container = container_for_song(song_uid);
      bool ss_deleted = false;
      if (!container.empty()) {
         ss_deleted = m_storage_system.delete_object(container, song_uid);
      }
      if (db_deleted && upload_metadata) {
         upload_metadata_db();
      }
      is_deleted = db_deleted || ss_deleted;
   }

   return is_deleted;
}

//*****************************************************************************

bool Jukebox::delete_artist(const string& artist) {
   bool is_deleted = false;
   if (!artist.empty()) {
      vector<SongMetadata> artist_song_list =
         m_jukebox_db->songs_for_artist(artist);
      if (artist_song_list.size() == 0) {
         printf("no artist songs in jukebox\n");
         return false;
      } else {
         for (const auto& song : artist_song_list) {
            if (!delete_song(song.fm.object_name, false)) {
               printf("error deleting song %s\n", song.fm.object_name.c_str());
               return false;
            }
         }
         upload_metadata_db();
         is_deleted = true;
      }
   }

   return is_deleted;
}

//*****************************************************************************

bool Jukebox::delete_album(const string& album) {
   size_t pos_double_dash = album.find("--");
   if (pos_double_dash != string::npos) {
      string artist = album.substr(0, pos_double_dash);
      int num_chars = album.length() - pos_double_dash - 2;
      string album_name = album.substr(pos_double_dash+2, num_chars);
      vector<SongMetadata> list_album_songs =
         m_jukebox_db->retrieve_album_songs(artist, album_name);
      if (list_album_songs.size() > 0) {
         int num_songs_deleted = 0;
         for (const auto& song : list_album_songs) {
            printf("%s %s\n",
                   song.fm.container_name.c_str(),
                   song.fm.object_name.c_str());
            // delete each song audio file
            if (m_storage_system.delete_object(song.fm.container_name,
                                               song.fm.object_name)) {
               num_songs_deleted += 1;
               // delete song metadata
               m_jukebox_db->delete_song(song.fm.object_name);
            } else {
               printf("error: unable to delete song %s\n",
                      song.fm.object_name.c_str());
               //FUTURE: delete song metadata if we got 404? (delete_album)
            }
         }
         if (num_songs_deleted > 0) {
            // upload metadata db
            upload_metadata_db();
            return true;
         }
      } else {
         printf("no songs found for artist=%s album name=%s\n",
                artist.c_str(),
                album_name.c_str());
      }
   } else {
      printf("specify album with 'the-artist--the-song-name' format\n");
   }
   return false;
}

//*****************************************************************************

bool Jukebox::delete_playlist(const string& playlist_name) {
   bool is_deleted = false;

   string object_file_name = playlist_name;
   if (!StrUtils::endsWith(object_file_name, JSON_FILE_EXT)) {
      object_file_name += JSON_FILE_EXT;
   }

   if (m_storage_system.delete_object(m_playlist_container, object_file_name)) {
      is_deleted = true;
   } else {
      printf("error: object delete failed\n");
   }

   return is_deleted;
}

//*****************************************************************************

void Jukebox::import_album_art() {
   if (m_jukebox_db && m_jukebox_db->is_open()) {
      int file_import_count = 0;
      vector<string> dir_listing =
         OSUtils::listFilesInDirectory(m_album_art_import_dir);
      if (dir_listing.size() == 0) {
         printf("no album art found\n");
         return;
      }

      for (const auto& listing_entry : dir_listing) {
         string full_path =
            OSUtils::pathJoin(m_album_art_import_dir, listing_entry);
         // ignore it if it's not a file
         if (Utils::path_isfile(full_path)) {
            string object_name = listing_entry;
            ReadFileResults read_results = read_file_contents(full_path);
            if (read_results.read_success &&
                read_results.file_bytes.size() > 0) {
               if (m_storage_system.put_object(m_album_art_container,
                                               object_name,
                                               read_results.file_bytes)) {
                  file_import_count += 1;
               }
            }
         }
      }

      if (file_import_count > 0) {
         printf("%d album art files imported\n", file_import_count);
      } else {
         printf("no files imported\n");
      }
   }
}

//*****************************************************************************

void Jukebox::prepare_for_termination() {
   printf("Ctrl-C detected, shutting down\n");

   // indicate that it's time to shutdown
   m_exit_requested = true;

   // terminate audio player if it's running
   if (m_audio_player_process > 0) {
      kill(m_audio_player_process, SIGTERM);
      m_audio_player_process = -1;
   }
}

//*****************************************************************************

void Jukebox::display_info() const {
   if (m_song_list.size() > 0) {
      long max_index = m_song_list.size() - 1;
      if (m_song_index + 3 <= max_index) {
         printf("----- songs on deck -----\n");
         const SongMetadata& first_song = m_song_list[m_song_index+1];
         printf("%s\n", first_song.fm.file_uid.c_str());
         const SongMetadata& second_song = m_song_list[m_song_index+2];
         printf("%s\n", second_song.fm.file_uid.c_str());
         const SongMetadata& third_song = m_song_list[m_song_index+3];
         printf("%s\n", third_song.fm.file_uid.c_str());
         printf("-------------------------\n");
      }
   }
}

//*****************************************************************************

bool Jukebox::initialize_storage_system(StorageSystem& storage_sys,
                                        string prefix) {
   // create the containers that will hold songs
   char artist_song_chars[] = "0123456789abcdefghijklmnopqrstuvwxyz";
   const int num_chars = strlen(artist_song_chars);
   char buf_cnr_name[15]; // x-artist-songs

   strncpy(buf_cnr_name, " -artist-songs", 15);

   for (int i = 0; i < num_chars; i++) {
      char ch = artist_song_chars[i];
      buf_cnr_name[0] = ch;
      if (!storage_sys.create_container(buf_cnr_name)) {
         printf("error: unable to create container '%s'\n", buf_cnr_name);
         return false;
      }
   }

   // create the other (non-song) containers
   vector<string> cnr_names(4);
   cnr_names.push_back("music-metadata");
   cnr_names.push_back("album-art");
   cnr_names.push_back("albums");
   cnr_names.push_back("playlists");

   for (const auto& container_name : cnr_names) {
      if (!storage_sys.create_container(container_name)) {
         printf("error: unable to create container '%s'\n",
                container_name.c_str());
         return false;
      }
   }

   // delete metadata DB file if present
   string metadata_db_file = "jukebox_db.sqlite3";
   if (Utils::path_exists(metadata_db_file)) {
      //if (debug_print) {
      //   printf("deleting existing metadata DB file\n");
      //}
      OSUtils::deleteFile(metadata_db_file);
   }

   return true;
}

//*****************************************************************************

bool Jukebox::retrieve_album_track_object_list(const string& artist,
                                               const string& album,
                                               vector<string>& list_track_objects) {
   bool success = false;
   string json_file_name = JBUtils::encode_artist_album(artist, album) +
                           JSON_FILE_EXT;
   string local_json_file =
      OSUtils::pathJoin(m_song_play_dir, json_file_name);
   if (m_storage_system.get_object(m_album_container,
                                   json_file_name,
                                   local_json_file) > 0) {
      string album_json_contents;
      if (Utils::file_read_all_text(local_json_file, album_json_contents)) {
         if (!album_json_contents.empty()) {
            json album_json = json::parse(album_json_contents);
            if (album_json.contains("tracks")) {
               auto& track_list = album_json["tracks"];

               for (auto& track : track_list) {
                  list_track_objects.push_back(track["object"].get<std::string>());
               }
               if (list_track_objects.size() > 0) {
                  success = true;
               }
            }
         } else {
            printf("error: album json file is empty %s\n",
                   local_json_file.c_str());
         }
      } else {
         printf("error: unable to read album json file %s\n",
                local_json_file.c_str());
      }
   } else {
      printf("Unable to retrieve '%s' from '%s'\n",
             json_file_name.c_str(),
             m_album_container.c_str());
   }
   return success;
}

//*****************************************************************************

bool Jukebox::is_exit_requested() const {
   return m_exit_requested;
}

//*****************************************************************************

