#ifndef JUKEBOX_H
#define JUKEBOX_H

#include <memory>
#include <string>
#include <vector>
#include <sys/types.h>
#include <unistd.h>

#include "jukebox_options.h"
#include "song_metadata.h"
#include "storage_system.h"
#include "PthreadsThread.h"
#include "Runnable.h"
#include "RunCompletionObserver.h"

class JukeboxDB;
class SongDownloader;


class ReadFileResults {
private:
   ReadFileResults();

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

   ReadFileResults(const ReadFileResults& copy) :
      read_success(copy.read_success),
      file_bytes(copy.file_bytes),
      num_file_bytes(copy.num_file_bytes) {
   }

   ReadFileResults(bool success) :
      read_success(success),
      num_file_bytes(0L) {
   }

   ~ReadFileResults() {
   }

   ReadFileResults& operator=(const ReadFileResults& copy) {
      if (this == &copy) {
         return *this;
      }

      read_success = copy.read_success;
      file_bytes = copy.file_bytes;
      num_file_bytes = copy.num_file_bytes;

      return *this;
   }
};


class Jukebox : public chaudiere::RunCompletionObserver {
private:
   std::unique_ptr<JukeboxDB> m_jukebox_db;
   std::unique_ptr<SongDownloader> m_downloader;
   std::unique_ptr<chaudiere::PthreadsThread> m_download_thread;
   JukeboxOptions m_jukebox_options;
   StorageSystem& m_storage_system;
   bool m_debug_print;
   std::string m_current_dir;
   std::string m_song_import_dir;
   std::string m_playlist_import_dir;
   std::string m_song_play_dir;
   std::string m_album_art_import_dir;
   std::string m_download_extension;
   std::string m_metadata_db_file;
   std::string m_metadata_container;
   std::string m_playlist_container;
   std::string m_album_container;
   std::string m_album_art_container;
   std::vector<SongMetadata> m_song_list;
   int m_number_songs;
   int m_song_index;
   std::string m_audio_player_exe_file_name;
   std::string m_audio_player_command_args;
   std::string m_audio_player_resume_args;
   pid_t m_audio_player_process;
   int m_cumulative_download_bytes;
   double m_cumulative_download_time;
   bool m_exit_requested;
   bool m_is_paused;
   double m_song_start_time;
   int m_song_seconds_offset;
   bool m_player_active;
   bool m_downloader_ready_to_delete;
   int m_num_successive_play_failures;
   bool m_song_play_is_resume;
   bool m_is_repeat_mode;

   Jukebox(const Jukebox&);
   Jukebox& operator=(const Jukebox&);


public:
   Jukebox(const JukeboxOptions& jb_options,
           StorageSystem& storage_sys,
           bool debug_print = false);
   ~Jukebox();

   bool enter();
   void exit();

   void toggle_pause_play();
   void advance_to_next_song();

   std::string get_metadata_db_file_path();

   static std::vector<std::string> components_from_file_name(const std::string& file_name);
   static std::string artist_from_file_name(const std::string& file_name);
   static std::string album_from_file_name(const std::string& file_name);
   static std::string song_from_file_name(const std::string& file_name);

   static void string_to_vector(const std::string& s, std::vector<unsigned char>& v);
   static void vector_to_string(const std::vector<unsigned char>& v, std::string& s);

   bool store_song_metadata(const SongMetadata& fs_song);
   void get_encryptor();

   std::string get_container_suffix();
   std::string object_file_suffix();
   std::string container_for_song(const std::string& song_uid);

   void import_songs();

   std::string song_path_in_playlist(const SongMetadata& song);

   bool check_file_integrity(const SongMetadata& song);

   void batch_download_start();
   void batch_download_complete();

   virtual void notifyRunComplete(chaudiere::Runnable* runnable);

   bool download_song(const SongMetadata& song);
   void play_song(const SongMetadata& song);
   void download_songs();
   void downloader_cleanup();
   void play_retrieved_songs(bool shuffle);
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
   bool get_playlist_songs(const std::string& playlist_name,
                           std::vector<SongMetadata>& list_songs);
   void show_album(const std::string& artist, const std::string& album);
   void show_playlist(const std::string& playlist);
   void play_playlist(const std::string& playlist);

   bool delete_song(const std::string& song_uid, bool upload_metadata=true);
   bool delete_artist(const std::string& artist);
   bool delete_album(const std::string& album);
   bool delete_playlist(const std::string& playlist_name);
   void import_album_art();
   void prepare_for_termination();
   void display_info() const;

   static bool initialize_storage_system(StorageSystem& storage_sys, std::string prefix="");
   bool retrieve_album_track_object_list(const std::string& artist,
                                         const std::string& album,
                                         std::vector<std::string>& list_track_objects);
   bool is_exit_requested() const;
};

#endif


