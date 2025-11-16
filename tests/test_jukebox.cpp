#include "test_jukebox.h"
#include "jukebox.h"
#include "storage_system.h"

using namespace std;

class DummyStorageSystem : public StorageSystem {
public:
   DummyStorageSystem() :
      StorageSystem("Dummy") {
   }

   virtual ~DummyStorageSystem() {}

   virtual std::vector<std::string> list_account_containers() {
      return vector<string>();
   }

   virtual bool create_container(const std::string& container_name) {
      return false;
   }

   virtual bool delete_container(const std::string& container_name) {
      return false;
   }

   virtual std::vector<std::string> list_container_contents(const std::string& container_name) {
      return vector<string>();
   }

   virtual bool get_object_metadata(const std::string& container_name,
                                    const std::string& object_name,
                                    PropertySet& dict_props) {
      return false;
   }

   virtual bool put_object(const std::string& container_name,
                           const std::string& object_name,
                           const std::vector<unsigned char>& object_bytes,
                           const PropertySet* headers=nullptr) {
      return false;
   }

   virtual bool put_object_from_file(const std::string& container_name,
                                     const std::string& object_name,
                                     const std::string& object_file_path,
                                     const PropertySet* headers=nullptr) {
      return false;
   }

   virtual bool delete_object(const std::string& container_name,
                              const std::string& object_name) {
      return false;
   }

   virtual int64_t get_object(const std::string& container_name,
                          const std::string& object_name,
                          const std::string& local_file_path) {
      return 0;
   }

   virtual bool enter() {
      return false;
   }

   virtual void exit() {
   }
};


TestJukebox::TestJukebox() :
   TestSuite("TestJukebox") {
}

void TestJukebox::runTests() {
   test_enter();
   test_exit();
   test_toggle_pause_play();
   test_advance_to_next_song();
   test_get_metadata_db_file_path();
   test_components_from_file_name();
   test_artist_from_file_name();
   test_album_from_file_name();
   test_song_from_file_name();
   test_store_song_metadata();
   test_store_song_playlist();
   test_get_encryptor();
   test_get_container_suffix();
   test_object_file_suffix();
   test_container_for_song();
   test_import_songs();
   test_song_path_in_playlist();
   test_check_file_integrity();
   test_batch_download_start();
   test_batch_download_complete();
   test_download_song();
   test_play_song();
   test_download_songs();
   test_play_songs();
   test_show_list_containers();
   test_show_listings();
   test_show_artists();
   test_show_genres();
   test_show_albums();
   test_read_file_contents();
   test_upload_metadata_db();
   test_import_playlists();
   test_show_playlists();
   test_show_playlist();
   test_play_playlist();
   test_delete_song();
   test_delete_artist();
   test_delete_album();
   test_delete_playlist();
   test_import_album_art();
   test_initialize_storage_system();
}

void TestJukebox::test_enter() {
   TEST_CASE("test_enter");
   //TODO: implement test_enter
}

void TestJukebox::test_exit() {
   TEST_CASE("test_exit");
   //TODO: implement test_exit
}

void TestJukebox::test_toggle_pause_play() {
   TEST_CASE("test_toggle_pause_play");
   //TODO: implement test_toggle_pause_play
}

void TestJukebox::test_advance_to_next_song() {
   TEST_CASE("test_advance_to_next_song");
   //TODO: implement test_advance_to_next_song
}

void TestJukebox::test_get_metadata_db_file_path() {
   TEST_CASE("test_get_metadata_db_file_path");
   //TODO: implement test_get_metadata_db_file_path
}

void TestJukebox::test_components_from_file_name() {
   TEST_CASE("test_components_from_file_name");
   string file_name = "The-Who--Whos-Next--My-Wife.flac";
   vector<string> components;
   Jukebox::components_from_file_name(file_name, components);
   require(components.size() == 3, "file name has 3 components");
   if (components.size() == 3) {
      requireStringEquals("The Who", components[0]);
      requireStringEquals("Whos Next", components[1]);
      requireStringEquals("My Wife", components[2]);
   }
}

void TestJukebox::test_artist_from_file_name() {
   TEST_CASE("test_artist_from_file_name");
   string file_name = "The-Who--Whos-Next--My-Wife.flac";
   string artist_name = Jukebox::artist_from_file_name(file_name);
   requireStringEquals("The Who", artist_name);
}

void TestJukebox::test_album_from_file_name() {
   TEST_CASE("test_album_from_file_name");
   string file_name = "The-Who--Whos-Next--My-Wife.flac";
   string album_name = Jukebox::album_from_file_name(file_name);
   requireStringEquals("Whos Next", album_name);
}

void TestJukebox::test_song_from_file_name() {
   TEST_CASE("test_song_from_file_name");
   string file_name = "The-Who--Whos-Next--My-Wife.flac";
   string song_name = Jukebox::song_from_file_name(file_name);
   requireStringEquals("My Wife", song_name);
}

void TestJukebox::test_store_song_metadata() {
   TEST_CASE("test_store_song_metadata");
   //TODO: implement test_store_song_metadata
}

void TestJukebox::test_store_song_playlist() {
   TEST_CASE("test_store_song_playlist");
   //TODO: implement test_store_song_playlist
}

void TestJukebox::test_get_encryptor() {
   TEST_CASE("test_get_encryptor");
   //TODO: implement test_get_encryptor
}

void TestJukebox::test_get_container_suffix() {
   TEST_CASE("test_get_container_suffix");

   DummyStorageSystem dss;

   // no encryption, no compression
   {
      JukeboxOptions jb_options;
      Jukebox jukebox(jb_options, dss);
      string suffix = jukebox.get_container_suffix();
      requireStringEquals("", suffix);
   }

   // no encryption, yes compression
   {
      JukeboxOptions jb_options;
      jb_options.set_use_encryption(false);
      jb_options.set_use_compression(true);
      Jukebox jukebox(jb_options, dss);
      string suffix = jukebox.get_container_suffix();
      requireStringEquals("-z", suffix);
   }

   // yes encryption, no compression
   {
      JukeboxOptions jb_options;
      jb_options.set_use_encryption(true);
      jb_options.set_use_compression(false);
      jb_options.set_encryption_key("asdf");
      Jukebox jukebox(jb_options, dss);
      string suffix = jukebox.get_container_suffix();
      requireStringEquals("-e", suffix);
   }

   // yes encryption, yes compression
   {
      JukeboxOptions jb_options;
      jb_options.set_use_encryption(true);
      jb_options.set_use_compression(true);
      jb_options.set_encryption_key("asdf");
      Jukebox jukebox(jb_options, dss);
      string suffix = jukebox.get_container_suffix();
      requireStringEquals("-ez", suffix);
   }
}

void TestJukebox::test_object_file_suffix() {
   TEST_CASE("test_object_file_suffix");
   DummyStorageSystem dss;

   // no encryption, no compression
   {  
      JukeboxOptions jb_options;
      Jukebox jukebox(jb_options, dss);
      string suffix = jukebox.object_file_suffix();
      requireStringEquals("", suffix);
   }

   // no encryption, yes compression
   {
      JukeboxOptions jb_options;
      jb_options.set_use_encryption(false);
      jb_options.set_use_compression(true);
      Jukebox jukebox(jb_options, dss);
      string suffix = jukebox.object_file_suffix();
      requireStringEquals(".gz", suffix);
   }

   // yes encryption, no compression
   {
      JukeboxOptions jb_options;
      jb_options.set_use_encryption(true);
      jb_options.set_use_compression(false);
      jb_options.set_encryption_key("asdf");
      Jukebox jukebox(jb_options, dss);
      string suffix = jukebox.object_file_suffix();
      requireStringEquals(".e", suffix);
   }

   // yes encryption, yes compression
   {
      JukeboxOptions jb_options;
      jb_options.set_use_encryption(true);
      jb_options.set_use_compression(true);
      jb_options.set_encryption_key("asdf");
      Jukebox jukebox(jb_options, dss);
      string suffix = jukebox.object_file_suffix();
      requireStringEquals(".egz", suffix);
   }
}

void TestJukebox::test_container_for_song() {
   TEST_CASE("test_container_for_song");

   DummyStorageSystem dss;
   string container;

   // no encryption, no compression
   {  
      JukeboxOptions jb_options;
      Jukebox jukebox(jb_options, dss);
      container = jukebox.container_for_song("The-Who--Whos-Next--My-Wife");
      requireStringEquals("w-artist-songs", container);
      container = jukebox.container_for_song("A-Flock-of-Seagulls--Best-Of--I-Ran.flac");
      requireStringEquals("f-artist-songs", container);
      container = jukebox.container_for_song("10000-Maniacs--In-My-Tribe--Gun-Shy.flac");
      requireStringEquals("1-artist-songs", container);
      container = jukebox.container_for_song("Kansas--Leftoverture--The-Wall.mp3");
      requireStringEquals("k-artist-songs", container);
   }
}

void TestJukebox::test_import_songs() {
   TEST_CASE("test_import_songs");
   //TODO: implement test_import_songs
}

void TestJukebox::test_song_path_in_playlist() {
   TEST_CASE("test_song_path_in_playlist");
   //TODO: implement test_song_path_in_playlist
}

void TestJukebox::test_check_file_integrity() {
   TEST_CASE("test_check_file_integrity");
   //TODO: implement test_check_file_integrity
}

void TestJukebox::test_batch_download_start() {
   TEST_CASE("test_batch_download_start");
   //TODO: implement test_batch_download_start
}

void TestJukebox::test_batch_download_complete() {
   TEST_CASE("test_batch_download_complete");
   //TODO: implement test_batch_download_complete
}

void TestJukebox::test_download_song() {
   TEST_CASE("test_download_song");
   //TODO: implement test_download_song
}

void TestJukebox::test_play_song() {
   TEST_CASE("test_play_song");
   //TODO: implement test_play_song
}

void TestJukebox::test_download_songs() {
   TEST_CASE("test_download_songs");
   //TODO: implement test_download_songs
}

void TestJukebox::test_play_songs() {
   TEST_CASE("test_play_songs");
   //TODO: implement test_play_songs
}

void TestJukebox::test_show_list_containers() {
   TEST_CASE("test_show_list_containers");
   //TODO: implement test_show_list_containers
}

void TestJukebox::test_show_listings() {
   TEST_CASE("test_show_listings");
   //TODO: implement test_show_listings
}

void TestJukebox::test_show_artists() {
   TEST_CASE("test_show_artists");
   //TODO: implement test_show_artists
}

void TestJukebox::test_show_genres() {
   TEST_CASE("test_show_genres");
   //TODO: implement test_show_genres
}

void TestJukebox::test_show_albums() {
   TEST_CASE("test_show_albums");
   //TODO: implement test_show_albums
}

void TestJukebox::test_read_file_contents() {
   TEST_CASE("test_read_file_contents");
   //TODO: implement test_read_file_contents
}

void TestJukebox::test_upload_metadata_db() {
   TEST_CASE("test_upload_metadata_db");
   //TODO: implement test_upload_metadata_db
}

void TestJukebox::test_import_playlists() {
   TEST_CASE("test_import_playlists");
   //TODO: implement test_import_playlists
}

void TestJukebox::test_show_playlists() {
   TEST_CASE("test_show_playlists");
   //TODO: implement test_show_playlists
}

void TestJukebox::test_show_playlist() {
   TEST_CASE("test_show_playlist");
   //TODO: implement test_show_playlist
}

void TestJukebox::test_play_playlist() {
   TEST_CASE("test_play_playlist");
   //TODO: implement test_play_playlist
}

void TestJukebox::test_delete_song() {
   TEST_CASE("test_delete_song");
   //TODO: implement test_delete_song
}

void TestJukebox::test_delete_artist() {
   TEST_CASE("test_delete_artist");
   //TODO: implement test_delete_artist
}

void TestJukebox::test_delete_album() {
   TEST_CASE("test_delete_album");
   //TODO: implement test_delete_album
}

void TestJukebox::test_delete_playlist() {
   TEST_CASE("test_delete_playlist");
   //TODO: implement test_delete_playlist
}

void TestJukebox::test_import_album_art() {
   TEST_CASE("test_import_album_art");
   //TODO: implement test_import_album_art
}

void TestJukebox::test_initialize_storage_system() {
   TEST_CASE("test_initialize_storage_system");
   //TODO: implement test_initialize_storage_system
}

