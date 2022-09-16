#ifndef TEST_JUKEBOX_H
#define TEST_JUKEBOX_H

#include <string>
#include "TestSuite.h"


class TestJukebox : public poivre::TestSuite {
protected:
   void runTests();

   void test_enter();
   void test_exit();

   void test_toggle_pause_play();
   void test_advance_to_next_song();

   void test_get_metadata_db_file_path();

   void test_components_from_file_name();
   void test_artist_from_file_name();
   void test_album_from_file_name();
   void test_song_from_file_name();

   void test_store_song_metadata();
   void test_store_song_playlist();
   void test_get_encryptor();

   void test_get_container_suffix();
   void test_object_file_suffix();
   void test_container_for_song();

   void test_import_songs();

   void test_song_path_in_playlist();

   void test_check_file_integrity();

   void test_batch_download_start();
   void test_batch_download_complete();

   void test_download_song();
   void test_play_song();
   void test_download_songs();
   void test_play_songs();

   void test_show_list_containers();
   void test_show_listings();
   void test_show_artists();
   void test_show_genres();
   void test_show_albums();

   void test_read_file_contents();

   void test_upload_metadata_db();

   void test_import_playlists();
   void test_show_playlists();
   void test_show_playlist();
   void test_play_playlist();

   void test_delete_song();
   void test_delete_artist();
   void test_delete_album();
   void test_delete_playlist();
   void test_import_album_art();

   void test_initialize_storage_system();

public:
   TestJukebox();
   
};


#endif

