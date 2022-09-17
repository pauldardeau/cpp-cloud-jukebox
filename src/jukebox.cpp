#include <string.h>

#include "jukebox.h"
#include "jukebox_db.h"
#include "file_metadata.h"
#include "song_metadata.h"
#include "jb_utils.h"
#include "utils.h"
#include "OSUtils.h"
#include "StringTokenizer.h"
#include "StrUtils.h"

using namespace std;

/*
def signal_handler(signum: int, frame):
    if signum == signal.SIGUSR1:
        if g_jukebox_instance != NULL:
            g_jukebox_instance.toggle_pause_play()
    elif signum == signal.SIGUSR2:
        if g_jukebox_instance != NULL:
            g_jukebox_instance.advance_to_next_song()


def install_signal_handlers():
    if os.name == 'posix':
        signal.signal(signal.SIGUSR1, signal_handler)
        signal.signal(signal.SIGUSR2, signal_handler)
*/



Jukebox* Jukebox::g_jukebox_instance = NULL;

Jukebox::Jukebox(const JukeboxOptions& jb_options,
                 StorageSystem& storage_sys,
                 bool debugging) :
   jukebox_options(jb_options),
   storage_system(storage_sys),
   debug_print(debugging),
   jukebox_db(NULL),
   download_extension(".download"),
   metadata_db_file("jukebox_db.sqlite3"),
   metadata_container("music-metadata"),
   playlist_container("playlists"),
   album_art_container("album-art"),
   number_songs(0),
   song_index(-1),
   song_play_length_seconds(20),
   cumulative_download_bytes(0),
   cumulative_download_time(0.0),
   exit_requested(false),
   is_paused(false),
   song_start_time(0.0),
   song_seconds_offset(0)
{
   g_jukebox_instance = this;

   current_dir = chaudiere::OSUtils::getCurrentDirectory();
   song_import_dir = chaudiere::OSUtils::pathJoin(current_dir, "song-import");
   playlist_import_dir = chaudiere::OSUtils::pathJoin(current_dir, "playlist-import");
   song_play_dir = chaudiere::OSUtils::pathJoin(current_dir, "song-play");
   album_art_import_dir = chaudiere::OSUtils::pathJoin(current_dir, "album-art-import");
   //audio_player_process = NULL;

   if (jb_options.debug_mode) {
      debug_print = true;
   }

   if (debug_print) {
      printf("current_dir = %s\n", current_dir.c_str());
      printf("song_import_dir = %s\n", song_import_dir.c_str());
      printf("song_play_dir = %s\n", song_play_dir.c_str());
   }
}

bool Jukebox::enter() {
   if (debug_print) {
      printf("Jukebox.enter\n");
   }

   // look for stored metadata in the storage system
   if (storage_system.has_container(metadata_container) &&
       !jukebox_options.suppress_metadata_download) {

      // metadata container exists, retrieve container listing
      vector<string> container_contents = storage_system.list_container_contents(metadata_container);

      bool metadata_db_file_found = false;
      auto it_vec = container_contents.begin();
      const auto it_vec_end = container_contents.end();
      for (; it_vec != it_vec_end; it_vec++) {
         if (*it_vec == metadata_db_file) {
            metadata_db_file_found = true;
            break;
         }
      }

      // does our metadata DB file exist in the metadata container?
      if (metadata_db_file_found) {
         printf("metadata DB file exists in container, attempting to download\n");

         // download it
         string metadata_db_file_path = get_metadata_db_file_path();
         string download_file = metadata_db_file_path + ".download";

         printf("downloading metadata DB to %s\n", download_file.c_str());
         if (storage_system.get_object(metadata_container, metadata_db_file, download_file) > 0) {
            // have an existing metadata DB file?
            if (Utils::path_exists(metadata_db_file_path)) {
               if (debug_print) {
                  printf("deleting existing metadata DB file\n");
               }
               chaudiere::OSUtils::deleteFile(metadata_db_file_path);
            }
            // rename downloaded file
            if (debug_print) {
               printf("renaming %s to %s\n", download_file.c_str(), metadata_db_file_path.c_str());
            } 
            Utils::rename_file(download_file, metadata_db_file_path);
         } else {
            if (debug_print) {
               printf("error: unable to retrieve metadata DB file\n");
            }
         }
      } else {
         if (debug_print) {
            printf("no metadata DB file in metadata container\n");
         }
      }
   } else {
      if (debug_print) {
         printf("no metadata container in storage system\n");
      }
   }

   jukebox_db = new JukeboxDB(get_metadata_db_file_path());
   if (!jukebox_db->open()) {
      printf("unable to connect to database\n");
      return false;
   }

   return true;
}

void Jukebox::exit() {
   if (debug_print) {
      printf("Jukebox.exit\n");
   }

   if (jukebox_db != NULL) {
      if (jukebox_db->is_open()) {
         jukebox_db->close();
      }
      jukebox_db = NULL;
   }
}

void Jukebox::toggle_pause_play() {
   is_paused = !is_paused;
   if (is_paused) {
      printf("paused\n");
      //if (audio_player_process != NULL) {
         // capture current song position (seconds into song)
      //   audio_player_process.Kill();
      //}
   } else {
      printf("resuming play\n");
   }
}

void Jukebox::advance_to_next_song() {
   printf("advancing to next song\n");
   //if (audio_player_process != NULL) {
   //   audio_player_process.Kill();
   //}
}

string Jukebox::get_metadata_db_file_path() {
   return chaudiere::OSUtils::pathJoin(current_dir, metadata_db_file);
}

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

   vector<string> tokens = chaudiere::StrUtils::split(base_file_name, "--");
   if (tokens.size() == 3) {
      vector<string> ret_vet;
      ret_vet.push_back(JBUtils::unencode_value(tokens[0]));
      ret_vet.push_back(JBUtils::unencode_value(tokens[1]));
      ret_vet.push_back(JBUtils::unencode_value(tokens[2]));
      return ret_vet;
   } else {
      return vector<string>();
   }
}

string Jukebox::artist_from_file_name(const string& file_name) {
   if (file_name.length() > 0) {
      vector<string> components = components_from_file_name(file_name);
      if (components.size() == 3) {
         return components[0];
      }
   }
   return string("");
}

string Jukebox::album_from_file_name(const string& file_name) {
   if (file_name.length() > 0) {
      vector<string> components = components_from_file_name(file_name);
      if (components.size() == 3) {
         return components[1];
      }
   }
   return string("");
}

string Jukebox::song_from_file_name(const string& file_name) {
   if (file_name.length() > 0) {
      vector<string> components = components_from_file_name(file_name);
      if (components.size() == 3) {
         return components[2];
      }
   }
   return string("");
}

bool Jukebox::store_song_metadata(const SongMetadata& fs_song) {
   SongMetadata* db_song = jukebox_db->retrieve_song(fs_song.fm.file_uid);
   if (db_song != NULL) {
      if (fs_song != *db_song) {
         return jukebox_db->update_song(fs_song);
      } else {
         return true;  // no insert or update needed (already up-to-date)
      }
   } else {
      // song is not in the database, insert it
      return jukebox_db->insert_song(fs_song);
   }
}

bool Jukebox::store_song_playlist(const string& file_name, const string& file_contents) {
   //TODO: (2) json deserialization (store_song_playlist)
   //pl = json.loads(file_contents);
   //if ("name" in pl.keys()) {
   //    string pl_name = pl["name"];
   //    string pl_uid = file_name;
   //    return jukebox_db->insert_playlist(pl_uid, pl_name);
   //} else {
   //   return false;
   //}
   return false;
}

void Jukebox::get_encryptor() {
   //TODO: (3) encryption (get_encryptor)
   // key_block_size = 16  // AES-128
   // key_block_size = 24  // AES-192
   //int key_block_size = 32;  // AES-256
   //return AESBlockEncryption(key_block_size,
   //                          jukebox_options.encryption_key,
   //                          jukebox_options.encryption_iv);
}

string Jukebox::get_container_suffix() {
   string suffix = "";
   if (jukebox_options.use_encryption && jukebox_options.use_compression) {
      suffix += "-ez";
   } else if (jukebox_options.use_encryption) {
      suffix += "-e";
   } else if (jukebox_options.use_compression) {
      suffix += "-z";
   }
   return suffix;
}

string Jukebox::object_file_suffix() {
   string suffix = "";
   if (jukebox_options.use_encryption && jukebox_options.use_compression) {
      suffix = ".egz";
   } else if (jukebox_options.use_encryption) {
      suffix = ".e";
   } else if (jukebox_options.use_compression) {
      suffix = ".gz";
   }
   return suffix;
}

string Jukebox::container_for_song(const string& song_uid) {
   if (song_uid.length() == 0) {
      return string("");
   }
   string container_suffix = "-artist-songs" + get_container_suffix();

   string artist = artist_from_file_name(song_uid);
   string artist_letter = "";
   if (chaudiere::StrUtils::startsWith(artist, "A ")) {
      artist_letter = artist.substr(2, 1);
   } else if (chaudiere::StrUtils::startsWith(artist, "The ")) {
      artist_letter = artist.substr(4, 1);
   } else {
      artist_letter = artist.substr(0, 1);
   }

   string container_name = artist_letter;
   chaudiere::StrUtils::toLowerCase(container_name);
   container_name += container_suffix;

   return container_name;
}

void Jukebox::import_songs() {
   if (jukebox_db != NULL && jukebox_db->is_open()) {
      vector<string> dir_listing = chaudiere::OSUtils::listFilesInDirectory(song_import_dir);
      float num_entries = (float) dir_listing.size();
      double progressbar_chars = 0.0;
      int progressbar_width = 40;
      int progress_chars_per_iteration = (int) (progressbar_width / num_entries);
      char progressbar_char = '#';
      int bar_chars = 0;

      if (!debug_print) {
         // setup progressbar
         /*
         string bar = new string('*', progressbar_width);
         string bar_text = "[" + bar + "]";
         Utils::sys_stdout_write(bar_text);
         Utils::sys_stdout_flush();
         bar = new string('\b', progressbar_width + 1);
         Utils::sys_stdout_write(bar);  // return to start of line, after '['
         */
      }

      //TODO: (3) encryption support (import_songs)
      //if (jukebox_options != NULL && jukebox_options.use_encryption) {
      //   encryption = get_encryptor();
      //} else {
      //   encryption = NULL;
      //}

      double cumulative_upload_time = 0.0;
      int cumulative_upload_bytes = 0;
      int file_import_count = 0;

      auto it = dir_listing.begin();
      const auto it_end = dir_listing.end();

      for (; it != it_end; it++) {
         const string& listing_entry = *it;
         string full_path = chaudiere::OSUtils::pathJoin(song_import_dir, listing_entry);
         // ignore it if it's not a file
         if (Utils::path_isfile(full_path)) {
            string file_name = listing_entry;
            vector<string> path_elems = Utils::path_splitext(full_path);
            const string& extension = path_elems[1];
            if (extension.length() > 0) {
               long file_size = Utils::get_file_size(full_path);
               string artist = artist_from_file_name(file_name);
               string album = album_from_file_name(file_name);
               string song = song_from_file_name(file_name);
               if (file_size > 0 && artist.length() > 0 && album.length() > 0 && song.length() > 0) {
                  string object_name = file_name + object_file_suffix();
                  SongMetadata fs_song;
                  fs_song.fm.file_uid = object_name;
                  fs_song.album_uid = "";
                  fs_song.fm.origin_file_size = (int) file_size;
                  fs_song.fm.file_time = Utils::datetime_datetime_fromtimestamp(Utils::path_getmtime(full_path));
                  fs_song.artist_name = artist;
                  fs_song.song_name = song;
                  fs_song.fm.md5_hash = Utils::md5_for_file(full_path);
                  fs_song.fm.compressed = jukebox_options.use_compression ? 1 : 0;
                  fs_song.fm.encrypted = jukebox_options.use_encryption ? 1 : 0;
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
                        if (jukebox_options.use_compression) {
                           if (debug_print) {
                              printf("compressing file\n");
                           }

                           //TODO: (3) compression (import_songs)
                           //file_bytes = bytes(file_contents, 'utf-8');
                           //file_contents = zlib.compress(file_bytes, 9);
                        }

                        if (jukebox_options.use_encryption) {
                           if (debug_print) {
                              printf("encrypting file\n");
                           }

                           //TODO: (3) encryption (import_songs)

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
                     if (storage_system.put_object(fs_song.fm.container_name,
                                                   fs_song.fm.object_name,
                                                   file_contents,
                                                   NULL)) {
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
                           printf("unable to store metadata, deleting obj %s\n", fs_song.fm.object_name.c_str());
                                              
                           storage_system.delete_object(fs_song.fm.container_name,
                                                        fs_song.fm.object_name);
                        } else {
                           file_import_count += 1;
                        }
                     } else {
                        printf("error: unable to upload %s to %s\n", fs_song.fm.object_name.c_str(), fs_song.fm.container_name.c_str());
                     }
                  }
               }
            }

            if (!debug_print) {
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

      if (!debug_print) {
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
      }

      printf("%d song files imported\n", file_import_count);

      if (cumulative_upload_time > 0) {
         double cumulative_upload_kb = cumulative_upload_bytes / 1000.0;
         int avg = (int) (cumulative_upload_kb / cumulative_upload_time);
         printf("average upload throughput = %d KB/sec\n", avg);
      }
   }
}

string Jukebox::song_path_in_playlist(const SongMetadata& song) {
   return chaudiere::OSUtils::pathJoin(song_play_dir, song.fm.file_uid);
}

bool Jukebox::check_file_integrity(const SongMetadata& song) {
   bool file_integrity_passed = true;

   if (jukebox_options.check_data_integrity) {
      string file_path = song_path_in_playlist(song);
      if (Utils::file_exists(file_path)) {
         if (debug_print) {
            printf("checking integrity for %s\n", song.fm.file_uid.c_str());
         }

         string playlist_md5 = Utils::md5_for_file(file_path);
         if (playlist_md5 == song.fm.md5_hash) {
            if (debug_print) {
               printf("integrity check SUCCESS\n");
            }
            file_integrity_passed = true;
         } else {
            printf("file integrity check failed: %s\n", song.fm.file_uid.c_str());
            file_integrity_passed = false;
         }
      } else {
         // file doesn't exist
         printf("file doesn't exist\n");
         file_integrity_passed = false;
      }
   } else {
      if (debug_print) {
         printf("file integrity bypassed, no jukebox options or check integrity not turned on\n");
      }
   }

   return file_integrity_passed;
}

void Jukebox::batch_download_start() {
   cumulative_download_bytes = 0;
   cumulative_download_time = 0.0;
}

void Jukebox::batch_download_complete() {
   if (!exit_requested) {
      if (cumulative_download_time > 0) {
         double cumulative_download_kb = cumulative_download_bytes / 1000.0;
         int avg = (int) (cumulative_download_kb / cumulative_download_time);
         printf("average download throughput = %d KB/sec\n", avg);
      }
      cumulative_download_bytes = 0;
      cumulative_download_time = 0.0;
   }
}

bool Jukebox::download_song(const SongMetadata& song) {
   if (exit_requested) {
      return false;
   }

   string file_path = song_path_in_playlist(song);
   double download_start_time = Utils::time_time();
   int song_bytes_retrieved = storage_system.retrieve_file(song.fm, song_play_dir);
   if (exit_requested) {
      return false;
   }

   if (debug_print) {
      printf("bytes retrieved: %d\n", song_bytes_retrieved);
   }

   if (song_bytes_retrieved > 0) {
      double download_end_time = Utils::time_time();
      double download_elapsed_time = download_end_time - download_start_time;
      cumulative_download_time += download_elapsed_time;
      cumulative_download_bytes += song_bytes_retrieved;

      // are we checking data integrity?
      // if so, verify that the storage system retrieved the same length that has been stored
      if (jukebox_options.check_data_integrity) {
         if (debug_print) {
            printf("verifying data integrity\n");
         }

         if (song_bytes_retrieved != song.fm.stored_file_size) {
            printf("error: data integrity check failed for %s\n", file_path.c_str());
            return false;
         }
      }

      // is it encrypted? if so, unencrypt it
      //int encrypted = song.fm.encrypted;
      //int compressed = song.fm.compressed;

      //TODO: (3) encryption and compression (download_song)
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
               return true;
      } else {
         // we retrieved the file, but it failed our integrity check
         // if file exists, remove it
         if (Utils::file_exists(file_path)) {
            chaudiere::OSUtils::deleteFile(file_path);
         }
      }
   }

   return false;
}

void Jukebox::play_song(const string& song_file_path) {
   if (Utils::path_exists(song_file_path)) {
      printf("playing %s\n", song_file_path.c_str());

      if (audio_player_command_args.length() > 0) {
         string cmd_args = audio_player_command_args + song_file_path;
         int exit_code = -1;
         bool started_audio_player = false;
         /*
         try
         {
            ProcessStartInfo psi = new ProcessStartInfo();
            psi.FileName = audio_player_exe_file_name;
            psi.Arguments = cmd_args;
            psi.UseShellExecute = false;
            psi.RedirectStandardError = false;
            psi.RedirectStandardOutput = false;

            audio_player_process = new Process();
            audio_player_process.StartInfo = psi;
            audio_player_process.Start();

            if (audio_player_process != NULL) {
               started_audio_player = true;
               song_start_time = Utils::time_time();
               audio_player_process.WaitForExit();
               if (audio_player_process.HasExited) {
                  exit_code = audio_player_process.ExitCode;
               }
               audio_player_process = NULL;
            }
         }
         catch (Exception)
         {
            // audio player not available
            audio_player_exe_file_name = "";
            audio_player_command_args = "";
            //audio_player_process = NULL;
            exit_code = -1;
         }
         */

         // if the audio player failed or is not present, just sleep
         // for the length of time that audio would be played
         if (!started_audio_player && exit_code != 0) {
            Utils::time_sleep(song_play_length_seconds);
         }
      } else {
         // we don't know about an audio player, so simulate a
         // song being played by sleeping
         Utils::time_sleep(song_play_length_seconds);
      }

      if (!is_paused) {
         // delete the song file from the play list directory
         chaudiere::OSUtils::deleteFile(song_file_path);
      }
   } else {
      printf("song file doesn't exist: %s\n", song_file_path.c_str());
      Utils::file_append_all_text("404.txt", song_file_path);
   }
}

void Jukebox::download_songs() {
   // scan the play list directory to see if we need to download more songs
   vector<string> dir_listing = chaudiere::OSUtils::listFilesInDirectory(song_play_dir);
   unsigned int song_file_count = 0;
   auto it = dir_listing.begin();
   const auto it_end = dir_listing.end();
   for (; it != it_end; it++) {
      const string& listing_entry = *it;
      string full_path = chaudiere::OSUtils::pathJoin(song_play_dir, listing_entry);
      if (Utils::path_isfile(full_path)) {
         vector<string> path_elems = Utils::path_splitext(full_path);
         const string& extension = path_elems[1];
         if (extension.length() > 0 && extension != download_extension) {
            song_file_count += 1;
         }
      }
   }

   unsigned int file_cache_count = jukebox_options.file_cache_count;

   if (song_file_count < file_cache_count) {
      vector<SongMetadata> dl_songs;
      // start looking at the next song in the list
      int check_index = song_index + 1;

      printf("DEBUG: number_songs = %d\n", number_songs);
 
      for (int j = 0; j < number_songs; j++) {
         printf("DEBUG: j = %d\n", j);

         if (check_index >= number_songs) {
            check_index = 0;
         }
         if (check_index != song_index) {
            SongMetadata si = song_list[check_index];
            string file_path = song_path_in_playlist(si);
            if (!Utils::file_exists(file_path)) {
               printf("adding song to dl_songs\n");
               dl_songs.push_back(si);
               if (dl_songs.size() >= file_cache_count) {
                  printf("DEBUG: dl_songs.size >= file_cache_count, breaking\n");
                  break;
               }
            }
         }
         check_index++;
      }

      if (dl_songs.size() > 0) {
         printf("dl_songs.size = %ld, not starting thread\n", dl_songs.size());
         /*
         printf("creating SongDownloader\n");
         SongDownloader downloader = new SongDownloader(this, dl_songs);
         Thread download_thread = new Thread(new ThreadStart(downloader.run));
         printf("starting thread to download songs\n");
         download_thread.Start();
         */
      }
   }
}

void Jukebox::play_songs(bool shuffle, string artist, string album) {
   vector<SongMetadata*> song_list = jukebox_db->retrieve_album_songs(artist, album);
   if (song_list.size() > 0) {
      number_songs = song_list.size();

      if (number_songs == 0) {
         printf("no songs in jukebox\n");
         return;
      }

      // does play list directory exist?
      if (!chaudiere::OSUtils::directoryExists(song_play_dir)) {
         if (debug_print) {
            printf("song-play directory does not exist, creating it\n");
         }
         chaudiere::OSUtils::createDirectory(song_play_dir);
      } else {
         // play list directory exists, delete any files in it
         if (debug_print) {
            printf("deleting existing files in song-play directory\n");
         }

         vector<string> list_files = chaudiere::OSUtils::listFilesInDirectory(song_play_dir);
         auto it = list_files.begin();
         const auto it_end = list_files.end();
         for (; it != it_end; it++) {
            const string& theFile = *it;
            string file_path = chaudiere::OSUtils::pathJoin(song_play_dir, theFile);
            if (Utils::path_isfile(file_path)) {
               chaudiere::OSUtils::deleteFile(file_path);
            }
         }
      }

      int song_index = 0;
      //TODO: (2) set up signal handlers (play_songs)
      //install_signal_handlers();

#if defined(__APPLE__)
      audio_player_exe_file_name = "afplay";
      audio_player_command_args = "";
#elif defined(__linux__) || defined(__unix__)
      audio_player_exe_file_name = "mplayer";
      audio_player_command_args = "-novideo -nolirc -really-quiet ";
#elif defined(_WIN32)
      // we really need command-line support for /play and /close arguments. unfortunately,
      // this support used to be available in the built-in Windows Media Player, but is
      // no longer present.
      // audio_player_command_args = "C:\Program Files\Windows Media Player\wmplayer.exe ";
      //
      /*
      audio_player_exe_file_name = "C:\\Program Files\\MPC-HC\\mpc-hc64.exe";
      audio_player_command_args = "/play /close /minimized ";
      */
#else
      audio_player_exe_file_name = "";
      audio_player_command_args = "";
#endif

      printf("downloading first song...\n");

      if (shuffle) {
         //TODO: (1) implement shuffling list (play_songs)
         //song_list = random.sample(song_list, song_list.size());
      }

      try
      {
         if (download_song(*song_list[0])) {
            printf("first song downloaded. starting playing now.\n");

            // write PID to "jukebox.pid"
            int pid_value = Utils::get_pid();
            char pid_text[128];
            memset(pid_text, 0, sizeof(pid_text));
            snprintf(pid_text, 128, "%d", pid_value);
            string str_pid_text = pid_text;
            Utils::file_write_all_text("jukebox.pid", str_pid_text);

            while (!exit_requested) {
               if (!is_paused) {
                  printf("DEBUG: calling download_songs\n");
                  download_songs();
                  printf("DEBUG: back from download_songs, calling play_song\n");
                  play_song(song_path_in_playlist(*song_list[song_index]));
               }
               if (!is_paused) {
                  song_index++;
                  if (song_index >= number_songs) {
                     song_index = 0;
                  }
               } else {
                  printf("DEBUG: is_paused, sleeping\n");
                  Utils::time_sleep(1);
               }
            }
            chaudiere::OSUtils::deleteFile("jukebox.pid");
         } else {
            printf("error: unable to download songs\n");
            Utils::sys_exit(1);
         }
      }
      catch (exception& e)
      {
         printf("exception caught: %s\n", e.what());
         printf("\nexiting jukebox\n");
         chaudiere::OSUtils::deleteFile("jukebox.pid");
         exit_requested = true;
      }
   }
}

void Jukebox::show_list_containers() {
   //TODO: implement show_list_containers
   /*
   if (storage_system.list_containers != NULL) {
      foreach (var container_name in storage_system.list_containers) {
         printf("%s\n", container_name.c_str());
      }
   }
   */
}

void Jukebox::show_listings() {
   if (jukebox_db != NULL) { 
      jukebox_db->show_listings();
   }
}

void Jukebox::show_artists() {
   if (jukebox_db != NULL) {
      jukebox_db->show_artists();
   }
}

void Jukebox::show_genres() {
   if (jukebox_db != NULL) { 
      jukebox_db->show_genres();
   }
}

void Jukebox::show_albums() {
   if (jukebox_db != NULL) {
      jukebox_db->show_albums();
   }
}

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
         if (jukebox_options.use_compression) {
            //TODO: (3) compression (read_file_contents)
     /*
            if (debug_print) {
               printf("compressing file\n");
            }

            file_bytes = bytes(file_contents, 'utf-8');
            file_contents = zlib.compress(file_bytes, 9);
      */
         }

         if (allow_encryption && jukebox_options.use_encryption) {
            //TODO: (3) encryption (read_file_contents)
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

bool Jukebox::upload_metadata_db() {
   bool metadata_db_upload = false;
   bool have_metadata_container = false;

   if (!storage_system.has_container(metadata_container)) {
      have_metadata_container = storage_system.create_container(metadata_container);
   } else {
      have_metadata_container = true;
   }

   if (have_metadata_container) {
      if (debug_print) {
         printf("uploading metadata db file to storage system\n");
      }

      jukebox_db->close();
      jukebox_db = NULL;

      // upload metadata DB file
      vector<unsigned char> db_file_contents;
      if (Utils::file_read_all_bytes(get_metadata_db_file_path(),
                                     db_file_contents)) {
         metadata_db_upload = storage_system.put_object(metadata_container,
                                                        metadata_db_file,
                                                        db_file_contents,
                                                        NULL);

         if (debug_print) {
            if (metadata_db_upload) {
               printf("metadata db file uploaded\n");
            } else {
               printf("unable to upload metadata db file\n");
            }
         }
      } else {
      }
   }

   return metadata_db_upload;
}

void Jukebox::import_playlists() {
   if (jukebox_db != NULL && jukebox_db->is_open()) {
      int file_import_count = 0;
      vector<string> dir_listing = chaudiere::OSUtils::listFilesInDirectory(playlist_import_dir);
      if (dir_listing.size() == 0) {
         printf("no playlists found\n");
         return;
      }

      bool have_container = false;

      if (!storage_system.has_container(playlist_container)) {
         have_container = storage_system.create_container(playlist_container);
      } else {
         have_container = true;
      }

      if (!have_container) {
         printf("error: unable to create container for playlists. unable to import\n");
         return;
      }

      auto it = dir_listing.begin();
      const auto it_end = dir_listing.end();

      for (; it != it_end; it++) {
         const string& listing_entry = *it;
         string full_path = chaudiere::OSUtils::pathJoin(playlist_import_dir, listing_entry);
         // ignore it if it's not a file
         if (Utils::file_exists(full_path)) {
            string object_name = listing_entry;
            //TODO: (2) read playlist (import_playlists)
            /*
            file_read, file_contents, _ = read_file_contents(full_path)
            if (file_read && file_contents != NULL) {
               if (storage_system.put_object(playlist_container,
                                             object_name,
                                             file_contents,
                                             NULL)) {
                  printf("put of playlist succeeded\n");
                  if (!store_song_playlist(object_name, file_contents)) {
                     printf("storing of playlist to db failed\n");
                     storage_system.delete_object(playlist_container, object_name);
                  } else {
                     printf("storing of playlist succeeded\n");
                     file_import_count += 1;
                  }
               }
            }
            */
         }
      }

      if (file_import_count > 0) {
         printf("%d playlists imported\n", file_import_count);
         // upload metadata DB file
         upload_metadata_db();
      } else {
         printf("no files imported\n");
      }
   }
}

void Jukebox::show_playlists() {
   if (jukebox_db != NULL) {
      jukebox_db->show_playlists();
   }
}

void Jukebox::show_playlist(const string& playlist) {
   printf("TODO: (2) implement (show_playlist)\n");
}

void Jukebox::play_playlist(const string& playlist) {
   printf("TODO: (2) implement (play_playlist)\n");
}

bool Jukebox::delete_song(const string& song_uid, bool upload_metadata) {
   bool is_deleted = false;
   if (song_uid.length() > 0) {
      bool db_deleted = jukebox_db->delete_song(song_uid);
      string container = container_for_song(song_uid);
      bool ss_deleted = false;
      if (container.length() > 0) {
         ss_deleted = storage_system.delete_object(container, song_uid);
      }
      if (db_deleted && upload_metadata) {
         upload_metadata_db();
      }
      is_deleted = db_deleted || ss_deleted;
   }

   return is_deleted;
}

bool Jukebox::delete_artist(const string& artist) {
   bool is_deleted = false;
   if (artist.length() > 0) {
      vector<SongMetadata*> song_list = jukebox_db->songs_for_artist(artist);
      if (song_list.size() == 0) {
         printf("no songs in jukebox\n");
         return false;
      } else {
         auto it = song_list.begin();
         const auto it_end = song_list.end();
         for (; it != it_end; it++) {
            SongMetadata* song = *it;
            if (!delete_song(song->fm.object_name, false)) {
               printf("error deleting song %s\n", song->fm.object_name.c_str());
               return false;
            }
         }
         upload_metadata_db();
         is_deleted = true;
      }
   }

   return is_deleted;
}

bool Jukebox::delete_album(const string& album) {
   size_t pos_double_dash = album.find("--");
   if (pos_double_dash != string::npos) {
      string artist = album.substr(0, pos_double_dash);
      int num_chars = album.length() - pos_double_dash - 2;
      string album_name = album.substr(pos_double_dash+2, num_chars);
      vector<SongMetadata*> list_album_songs = jukebox_db->retrieve_album_songs(artist, album_name);
      if (list_album_songs.size() > 0) {
         int num_songs_deleted = 0;
         auto it = list_album_songs.begin();
         const auto it_end = list_album_songs.end();
         for (; it != it_end; it++) {
            SongMetadata* song = *it;
            printf("%s %s\n", song->fm.container_name.c_str(), song->fm.object_name.c_str());
            // delete each song audio file
            if (storage_system.delete_object(song->fm.container_name, song->fm.object_name)) {
               num_songs_deleted += 1;
               // delete song metadata
               jukebox_db->delete_song(song->fm.object_name);
            } else {
               printf("error: unable to delete song %s\n", song->fm.object_name.c_str());
               //TODO: (3) delete song metadata if we got 404 (delete_album)
            }
         }
         if (num_songs_deleted > 0) {
            // upload metadata db
            upload_metadata_db();
            return true;
         }
      } else {
         printf("no songs found for artist=%s album name=%s\n", artist.c_str(), album_name.c_str());
      }
   } else {
      printf("specify album with 'the-artist--the-song-name' format\n");
   }
   return false;
}

bool Jukebox::delete_playlist(const string& playlist_name) {
   bool is_deleted = false;
   string object_name = jukebox_db->get_playlist(playlist_name);
   if (object_name.length() > 0) {
      bool db_deleted = jukebox_db->delete_playlist(playlist_name);
      if (db_deleted) {
         printf("container=%s, object=%s\n", playlist_container.c_str(), object_name.c_str());
         if (storage_system.delete_object(playlist_container, object_name)) {
            is_deleted = true;
         } else {
            printf("error: object delete failed\n");
         }
      } else {
         printf("error: database delete failed\n");
      }
      if (is_deleted) {
         upload_metadata_db();
      } else {
         printf("delete of playlist failed\n");
      }
   } else {
      printf("invalid playlist name\n");
   }

   return is_deleted;
}

void Jukebox::import_album_art() {
   if (jukebox_db != NULL && jukebox_db->is_open()) {
      int file_import_count = 0;
      vector<string> dir_listing = chaudiere::OSUtils::listFilesInDirectory(album_art_import_dir);
      if (dir_listing.size() == 0) {
         printf("no album art found\n");
         return;
      }

      bool have_container = false;

      if (!storage_system.has_container(album_art_container)) {
         have_container = storage_system.create_container(album_art_container);
      } else {
         have_container = true;
      }

      if (!have_container) {
         printf("error: unable to create container for album art. unable to import\n");
         return;
      }

      auto it = dir_listing.begin();
      const auto it_end = dir_listing.end();

      for (; it != it_end; it++) {
         const string& listing_entry = *it;
         string full_path = chaudiere::OSUtils::pathJoin(album_art_import_dir, listing_entry);
         // ignore it if it's not a file
         if (Utils::path_isfile(full_path)) {
            string object_name = listing_entry;
            //TODO: (3) album art import (import_album_art)
            /*
            file_read, file_contents, _ = read_file_contents(full_path);
            if (file_read && file_contents != NULL) {
               if (storage_system.put_object(album_art_container,
                                             object_name,
                                             file_contents)) {
                  file_import_count += 1;
               }
            }
            */
         }
      }

      if (file_import_count > 0) {
         printf("%d album art files imported\n", file_import_count);
      } else {
         printf("no files imported\n");
      }
   }
}

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
   vector<string> cnr_names;
   cnr_names.push_back("music-metadata");
   cnr_names.push_back("album-art");
   cnr_names.push_back("albums");
   cnr_names.push_back("playlists");

   auto it = cnr_names.begin();
   const auto it_end = cnr_names.end();

   for (; it != it_end; it++) {
      const string& container_name = *it;
      if (!storage_sys.create_container(container_name)) {
         printf("error: unable to create container '%s'\n", container_name.c_str());
	 return false;
      }
   }

   return true;
}

