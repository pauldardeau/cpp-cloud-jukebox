#include <string>

#include "test_jukebox_db.h"
#include "jukebox_db.h"
#include "fs_test_case.h"

using namespace std;

TestJukeboxDB::TestJukeboxDB() :
   TestSuite("TestJukeboxDB") {
}

void TestJukeboxDB::runTests() {
   test_is_open();
   test_open_db();
   test_open();
   test_close();
   test_create_table();
   test_create_tables();
   test_have_tables();
   test_get_playlist();
   test_songs_for_query();
   test_retrieve_song();
   test_insert_playlist();
   test_delete_playlist();
   test_insert_song();
   test_update_song();
   test_store_song_metadata();
   test_sql_where_clause();
   test_retrieve_album_songs();
   test_songs_for_artist();
   test_show_listings();
   test_show_artists();
   test_show_genres();
   test_show_artist_albums();
   test_show_albums();
   test_show_playlists();
   test_delete_song();
}

void TestJukeboxDB::test_is_open() {
   TEST_CASE("test_is_open");
   string test_dir = "/tmp/test_cpp_jukeboxdb_is_open";
   FSTestCase test_case(*this, test_dir);
   string db_file = "jukebox_db.sqlite3";
   JukeboxDB jbdb(db_file);
   requireFalse(jbdb.is_open(), "is_open must return false when DB is closed");
   require(jbdb.open(), "open must return true");
   require(jbdb.is_open(), "is_open must return true when DB is open");
   jbdb.close();
}

void TestJukeboxDB::test_open_db() {
   TEST_CASE("test_open_db");
   string test_dir = "/tmp/test_cpp_jukeboxdb_open_db";
   FSTestCase test_case(*this, test_dir);
   string db_file = "jukebox_db.sqlite3";
   JukeboxDB jbdb(db_file);
   require(jbdb.open_db(), "open_db must return true");
   jbdb.close();
}

void TestJukeboxDB::test_open() {
   TEST_CASE("test_open");
   string test_dir = "/tmp/test_cpp_jukeboxdb_open";
   FSTestCase test_case(*this, test_dir);
   string db_file = "jukebox_db.sqlite3";
   JukeboxDB jbdb(db_file);
   require(jbdb.open(), "open must return true");
   jbdb.close();
}

void TestJukeboxDB::test_close() {
   TEST_CASE("test_close");
   string test_dir = "/tmp/test_cpp_jukeboxdb_open";
   FSTestCase test_case(*this, test_dir);
   string db_file = "jukebox_db.sqlite3";
   JukeboxDB jbdb(db_file);
   jbdb.close();
   require(jbdb.open(), "open must return true");
   jbdb.close();
}

void TestJukeboxDB::test_create_table() {
   TEST_CASE("test_create_table");
   string test_dir = "/tmp/test_cpp_jukeboxdb_create_table";
   FSTestCase test_case(*this, test_dir);
   string db_file = "jukebox_db.sqlite3";
   JukeboxDB jbdb(db_file);

   // DB not open, valid sql
   string sql = "CREATE TABLE customer ("
                   "cust_uid TEXT UNIQUE NOT NULL,"
                   "cust_name TEXT NOT NULL,"
                   "cust_description TEXT)";
   requireFalse(jbdb.create_table(sql), "create table must return false for DB not opened");

   // DB open, valid sql
   require(jbdb.open(), "open must return true");
   require(jbdb.create_table(sql), "create table with valid sql must return true");

   // DB open, invalid sql
   string invalid_sql = "CREATE TABLE asdf "
                           "uid TEXTS UNIQUE NOT NULL,"
                           "name FOO NOT NULL"
                           "description TEXT)";
   requireFalse(jbdb.create_table(invalid_sql), "create table with invalid sql must return false");
   jbdb.close();
}

void TestJukeboxDB::test_create_tables() {
   TEST_CASE("test_create_tables");
   //TODO: implement test_create_tables
}

void TestJukeboxDB::test_have_tables() {
   TEST_CASE("test_have_tables");
   //TODO: implement test_have_tables
}

void TestJukeboxDB::test_get_playlist() {
   TEST_CASE("test_get_playlist");
   //TODO: implement test_get_playlist
}

void TestJukeboxDB::test_songs_for_query() {
   TEST_CASE("test_songs_for_query");
   //TODO: implement test_songs_for_query
}

void TestJukeboxDB::test_retrieve_song() {
   TEST_CASE("test_retrieve_song");
   //TODO: implement test_retrieve_song
}

void TestJukeboxDB::test_insert_playlist() {
   TEST_CASE("test_insert_playlist");
   //TODO: implement test_insert_playlist
}

void TestJukeboxDB::test_delete_playlist() {
   TEST_CASE("test_delete_playlist");
   //TODO: implement test_delete_playlist
}

void TestJukeboxDB::test_insert_song() {
   TEST_CASE("test_insert_song");
   string test_dir = "/tmp/test_cpp_jukeboxdb_insert_song";
   FSTestCase test_case(*this, test_dir);
   string db_file = "jukebox_db.sqlite3";
   JukeboxDB jbdb(db_file);
   require(jbdb.open(), "open must return true");

   SongMetadata song;
   song.fm.file_uid = "The-Who--Whos-Next--My-Wife.flac";
   song.fm.file_name = "The-Who--Whos-Next--My-Wife.flac";
   song.fm.origin_file_size = 23827669L;
   song.fm.stored_file_size = 23827669L;
   song.fm.pad_char_count = 0L;
   song.fm.file_time = "2022-09-17 08:56:0.000";
   song.fm.md5_hash = "asdf";
   song.fm.compressed = 0;
   song.fm.encrypted = 0;
   song.fm.container_name = "w-artist-songs";
   song.fm.object_name = "The-Who--Whos-Next--My-Wife.flac";
   song.artist_uid = "The-Who";
   song.artist_name = "The Who";
   song.album_uid = "Whos-Next";
   song.song_name = "My Wife";

   require(jbdb.insert_song(song), "insert_song must return true");

   requireFalse(jbdb.insert_song(song), "insert_song must return false on 2nd insert");
   jbdb.close();
}

void TestJukeboxDB::test_update_song() {
   TEST_CASE("test_update_song");
   //TODO: implement test_update_song
}

void TestJukeboxDB::test_store_song_metadata() {
   TEST_CASE("test_store_song_metadata");
   //TODO: implement test_store_song_metadata
}

void TestJukeboxDB::test_sql_where_clause() {
   TEST_CASE("test_sql_where_clause");
   //TODO: implement test_sql_where_clause
}

void TestJukeboxDB::test_retrieve_album_songs() {
   TEST_CASE("test_retrieve_album_songs");
   //TODO: implement test_retrieve_album_songs
}

void TestJukeboxDB::test_songs_for_artist() {
   TEST_CASE("test_songs_for_artist");
   //TODO: implement test_songs_for_artist
}

void TestJukeboxDB::test_show_listings() {
   TEST_CASE("test_show_listings");
   //TODO: implement test_show_listings
}

void TestJukeboxDB::test_show_artists() {
   TEST_CASE("test_show_artists");
   //TODO: implement test_show_artists
}

void TestJukeboxDB::test_show_genres() {
   TEST_CASE("test_show_genres");
   //TODO: implement test_show_genres
}

void TestJukeboxDB::test_show_artist_albums() {
   TEST_CASE("test_show_artist_albums");
   //TODO: implement test_show_artist_albums
}

void TestJukeboxDB::test_show_albums() {
   TEST_CASE("test_show_albums");
   //TODO: implement test_show_albums
}

void TestJukeboxDB::test_show_playlists() {
   TEST_CASE("test_show_playlists");
   //TODO: implement test_show_playlists
}

void TestJukeboxDB::test_delete_song() {
   TEST_CASE("test_delete_song");
   string test_dir = "/tmp/test_cpp_jukeboxdb_delete_song";
   FSTestCase test_case(*this, test_dir);
   string db_file = "jukebox_db.sqlite3";
   JukeboxDB jbdb(db_file);
   require(jbdb.open(), "open must return true");

   string song_uid = "The-Who--Whos-Next--My-Wife.flac";

   requireFalse(jbdb.delete_song(song_uid), "delete_song must return false for non-existing song");

   SongMetadata song;
   song.fm.file_uid = "The-Who--Whos-Next--My-Wife.flac";
   song.fm.file_name = "The-Who--Whos-Next--My-Wife.flac";
   song.fm.origin_file_size = 23827669L;
   song.fm.stored_file_size = 23827669L;
   song.fm.pad_char_count = 0L;
   song.fm.file_time = "2022-09-17 08:56:0.000";
   song.fm.md5_hash = "asdf";
   song.fm.compressed = 0;
   song.fm.encrypted = 0;
   song.fm.container_name = "w-artist-songs";
   song.fm.object_name = "The-Who--Whos-Next--My-Wife.flac";
   song.artist_uid = "The-Who";
   song.artist_name = "The Who";
   song.album_uid = "Whos-Next";
   song.song_name = "My Wife";

   require(jbdb.insert_song(song), "insert_song must return true");
   require(jbdb.delete_song(song_uid), "delete_song must return true for existing song");
   requireFalse(jbdb.delete_song(song_uid), "delete_song must return false on 2nd delete");

   jbdb.close();
}

