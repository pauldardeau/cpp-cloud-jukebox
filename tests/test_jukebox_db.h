#ifndef TEST_JUKEBOX_DB_H
#define TEST_JUKEBOX_DB_H

#include <string>
#include "TestSuite.h"

class JukeboxDB;

class TestJukeboxDB : public chaudiere::TestSuite {
protected:
   virtual void setup();
   virtual void tearDown();

   bool insert_album(JukeboxDB& jbdb);

   void runTests();

   void test_is_open();
   void test_open_db();
   void test_open();
   void test_close();

   void test_create_table();
   void test_create_tables();

   void test_have_tables();

   void test_get_playlist();

   void test_songs_for_query();

   void test_retrieve_song();
   void test_insert_playlist();
   void test_delete_playlist();
   void test_insert_song();
   void test_update_song();
   void test_store_song_metadata();
   void test_sql_where_clause();
   void test_retrieve_album_songs();
   void test_songs_for_artist();
   void test_show_listings();
   void test_show_artists();
   void test_show_genres();
   void test_show_artist_albums();
   void test_show_albums();
   void test_show_playlists();
   void test_delete_song();

public:
   TestJukeboxDB();
   
};


#endif

