#ifndef JUKEBOX_H
#define JUKEBOX_H

#include <string>
#include <vector>

#include "jukebox_options.h"
#include "song_metadata.h"
#include "storage_system.h"

class JukeboxDB;


class ReadFileResults {
private:
   ReadFileResults();
   ReadFileResults(const ReadFileResults&);
   ReadFileResults& operator=(const ReadFileResults&);

public:
   bool read_success;
   std::vector<unsigned char> file_bytes;
   long num_file_bytes;

   ReadFileResults(bool success,
		   const std::vector<unsigned char>& the_file_bytes,
		   long num_bytes) :
      read_success(success),
      file_bytes(the_file_bytes),
      num_file_bytes(num_bytes) {
   }

   ReadFileResults(bool success) :
      read_success(success),
      num_file_bytes(0L) {
   }

   ~ReadFileResults() {
   }
};


class Jukebox {
public:
   static Jukebox* g_jukebox_instance;

   JukeboxOptions jukebox_options;
   StorageSystem& storage_system;
   bool debug_print;
   JukeboxDB* jukebox_db;
   std::string current_dir;
   std::string song_import_dir;
   std::string playlist_import_dir;
   std::string song_play_dir;
   std::string album_art_import_dir;
   std::string download_extension;
   std::string metadata_db_file;
   std::string metadata_container;
   std::string playlist_container;
   std::string album_art_container;
   std::vector<SongMetadata> song_list;
   int number_songs;
   int song_index;
   std::string audio_player_exe_file_name;
   std::string audio_player_command_args;
   //Process audio_player_process;
   int song_play_length_seconds;
   int cumulative_download_bytes;
   double cumulative_download_time;
   bool exit_requested;
   bool is_paused;
   double song_start_time;
   int song_seconds_offset;

   Jukebox(const JukeboxOptions& jb_options,
           StorageSystem& storage_sys,
           bool debug_print = false);

   bool enter();
   void exit();

   void toggle_pause_play();
   void advance_to_next_song();

   std::string get_metadata_db_file_path();

   static std::vector<std::string> components_from_file_name(const std::string& file_name);
   static std::string artist_from_file_name(const std::string& file_name);
   static std::string album_from_file_name(const std::string& file_name);
   static std::string song_from_file_name(const std::string& file_name);

   bool store_song_metadata(const SongMetadata& fs_song);
   bool store_song_playlist(const std::string& file_name,
                            const std::string& file_contents);
   void get_encryptor();

   std::string get_container_suffix();
   std::string object_file_suffix();
   std::string container_for_song(const std::string& song_uid);

   void import_songs();

   std::string song_path_in_playlist(const SongMetadata& song);

   bool check_file_integrity(const SongMetadata& song);

   void batch_download_start();
   void batch_download_complete();

   bool download_song(const SongMetadata& song);
   void play_song(const std::string& song_file_path);
   void download_songs();
   void play_songs(bool shuffle=false,
		   std::string artist="",
		   std::string album="");

   void show_list_containers();
   void show_listings();
   void show_artists();
   void show_genres();
   void show_albums();

   ReadFileResults read_file_contents(const std::string& file_path,
                                      bool allow_encryption = true);

   bool upload_metadata_db();

   void import_playlists();
   void show_playlists();
   void show_playlist(const std::string& playlist);
   void play_playlist(const std::string& playlist);

   bool delete_song(const std::string& song_uid, bool upload_metadata=true);
   bool delete_artist(const std::string& artist);
   bool delete_album(const std::string& album);
   bool delete_playlist(const std::string& playlist_name);
   void import_album_art();

   static bool initialize_storage_system(StorageSystem& storage_sys, std::string prefix="");
};

#endif


