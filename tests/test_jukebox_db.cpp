#include <string>

#include "test_jukebox_db.h"
#include "jukebox_db.h"
#include "fs_test_case.h"
#include "OSUtils.h"

using namespace std;
using namespace chaudiere;

TestJukeboxDB::TestJukeboxDB() :
   TestSuite("TestJukeboxDB") {
}

void TestJukeboxDB::setup() {
   string fileName = "jukebox_db.sqlite3";
   if (OSUtils::pathExists(fileName)) {
      OSUtils::deleteFile(fileName);
   }
}

void TestJukeboxDB::tearDown() {
   string fileName = "jukebox_db.sqlite3";
   if (OSUtils::pathExists(fileName)) {
      OSUtils::deleteFile(fileName);
   }
}

bool TestJukeboxDB::insert_album(JukeboxDB& jbdb) {
   std::vector<SongMetadata> songs;

   std::string artist = "Steely-Dan";
   std::string album = "Aja";
   std::string prefix = artist + string("--") + album;
   std::string container = "s-artist-songs";

   {
      std::string song_name = "Black Cow";
      std::string song_file = prefix + "Black-Cow";

      SongMetadata song;
      song.set_file_uid(song_file);
      song.set_file_name(song_file);
      song.set_origin_file_size(23827669L);
      song.set_stored_file_size(23827669L);
      song.set_pad_char_count(0L);
      song.set_file_time("2022-09-17 08:56:0.000");
      song.set_md5_hash("asdf");
      song.set_compressed(0);
      song.set_encrypted(0);
      song.set_container_name(container);
      song.set_object_name(song_file);
      song.set_artist_uid(artist);
      song.set_artist_name(artist);
      song.set_album_uid(album);
      song.set_song_name(song_name);

      songs.push_back(song);
   }

   {
      std::string song_name = "Aja";
      std::string song_file = prefix + "Aja";

      SongMetadata song;
      song.set_file_uid(song_file);
      song.set_file_name(song_file);
      song.set_origin_file_size(23827669L);
      song.set_stored_file_size(23827669L);
      song.set_pad_char_count(0L);
      song.set_file_time("2022-09-17 08:56:0.000");
      song.set_md5_hash("asdf");
      song.set_compressed(0);
      song.set_encrypted(0);
      song.set_container_name(container);
      song.set_object_name(song_file);
      song.set_artist_uid(artist);
      song.set_artist_name(artist);
      song.set_album_uid(album);
      song.set_song_name(song_name);

      songs.push_back(song);
   }

   {
      std::string song_name = "Deacon Blues";
      std::string song_file = prefix + "Deacon-Blues";

      SongMetadata song;
      song.set_file_uid(song_file);
      song.set_file_name(song_file);
      song.set_origin_file_size(23827669L);
      song.set_stored_file_size(23827669L);
      song.set_pad_char_count(0L);
      song.set_file_time("2022-09-17 08:56:0.000");
      song.set_md5_hash("asdf");
      song.set_compressed(0);
      song.set_encrypted(0);
      song.set_container_name(container);
      song.set_object_name(song_file);
      song.set_artist_uid(artist);
      song.set_artist_name(artist);
      song.set_album_uid(album);
      song.set_song_name(song_name);

      songs.push_back(song);
   }

   {
      std::string song_name = "Peg";
      std::string song_file = prefix + "Peg";

      SongMetadata song;
      song.set_file_uid(song_file);
      song.set_file_name(song_file);
      song.set_origin_file_size(23827669L);
      song.set_stored_file_size(23827669L);
      song.set_pad_char_count(0L);
      song.set_file_time("2022-09-17 08:56:0.000");
      song.set_md5_hash("asdf");
      song.set_compressed(0);
      song.set_encrypted(0);
      song.set_container_name(container);
      song.set_object_name(song_file);
      song.set_artist_uid(artist);
      song.set_artist_name(artist);
      song.set_album_uid(album);
      song.set_song_name(song_name);

      songs.push_back(song);
   }

   {
      std::string song_name = "Home at Last";
      std::string song_file = prefix + "Home-at-Last";

      SongMetadata song;
      song.set_file_uid(song_file);
      song.set_file_name(song_file);
      song.set_origin_file_size(23827669L);
      song.set_stored_file_size(23827669L);
      song.set_pad_char_count(0L);
      song.set_file_time("2022-09-17 08:56:0.000");
      song.set_md5_hash("asdf");
      song.set_compressed(0);
      song.set_encrypted(0);
      song.set_container_name(container);
      song.set_object_name(song_file);
      song.set_artist_uid(artist);
      song.set_artist_name(artist);
      song.set_album_uid(album);
      song.set_song_name(song_name);

      songs.push_back(song);
   }

   {
      std::string song_name = "I Got the News";
      std::string song_file = prefix + "I-Got-the-News";

      SongMetadata song;
      song.set_file_uid(song_file);
      song.set_file_name(song_file);
      song.set_origin_file_size(23827669L);
      song.set_stored_file_size(23827669L);
      song.set_pad_char_count(0L);
      song.set_file_time("2022-09-17 08:56:0.000");
      song.set_md5_hash("asdf");
      song.set_compressed(0);
      song.set_encrypted(0);
      song.set_container_name(container);
      song.set_object_name(song_file);
      song.set_artist_uid(artist);
      song.set_artist_name(artist);
      song.set_album_uid(album);
      song.set_song_name(song_name);

      songs.push_back(song);
   }

   {
      std::string song_name = "Josie";
      std::string song_file = prefix + "Josie";

      SongMetadata song;
      song.set_file_uid(song_file);
      song.set_file_name(song_file);
      song.set_origin_file_size(23827669L);
      song.set_stored_file_size(23827669L);
      song.set_pad_char_count(0L);
      song.set_file_time("2022-09-17 08:56:0.000");
      song.set_md5_hash("asdf");
      song.set_compressed(0);
      song.set_encrypted(0);
      song.set_container_name(container);
      song.set_object_name(song_file);
      song.set_artist_uid(artist);
      song.set_artist_name(artist);
      song.set_album_uid(album);
      song.set_song_name(song_name);

      songs.push_back(song);
   }

   int songs_added = 0;
   for (const auto& song : songs) {
      if (jbdb.insert_song(song)) {
         songs_added++;
      }
   }

   return songs_added == songs.size();
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
   //printf("expecting failure below\n");
   requireFalse(jbdb.create_table(sql), "create table must return false for DB not opened");

   // DB open, valid sql
   require(jbdb.open(), "open must return true");
   require(jbdb.create_table(sql), "create table with valid sql must return true");

   // DB open, invalid sql
   string invalid_sql = "CREATE TABLE asdf "
                           "uid TEXTS UNIQUE NOT NULL,"
                           "name FOO NOT NULL"
                           "description TEXT)";
   //printf("expecting failure below\n");
   requireFalse(jbdb.create_table(invalid_sql), "create table with invalid sql must return false");
}

void TestJukeboxDB::test_create_tables() {
   TEST_CASE("test_create_tables");
   string test_dir = "/tmp/test_cpp_jukeboxdb_create_tables";
   FSTestCase test_case(*this, test_dir);
   string db_file = "jukebox_db.sqlite3";
   JukeboxDB jbdb(db_file);
   require(jbdb.open_db(), "open_db must return true");

   require(jbdb.create_tables(), "normal create_tables must return true");

   // 2nd attempt
   printf("=== expecting SQL error below\n");
   requireFalse(jbdb.create_tables(), "2nd call to create_tables must return false");
}

void TestJukeboxDB::test_have_tables() {
   TEST_CASE("test_have_tables");
   string test_dir = "/tmp/test_cpp_jukeboxdb_have_tables";
   FSTestCase test_case(*this, test_dir);
   string db_file = "jukebox_db.sqlite3";
   JukeboxDB jbdb(db_file);
   require(jbdb.open_db(), "open_db must return true");

   requireFalse(jbdb.have_tables(), "have_tables must return false before they're created");
   require(jbdb.create_tables(), "normal create_tables must return true");
   require(jbdb.have_tables(), "have_tables must return true when they exist");
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
   string test_dir = "/tmp/test_cpp_jukeboxdb_retrieve_song";
   FSTestCase test_case(*this, test_dir);
   string db_file = "jukebox_db.sqlite3";
   JukeboxDB jbdb(db_file);
   require(jbdb.open(), "open must return true");

   SongMetadata result_song;

   //TODO: insert some songs

   // empty song file
   bool success = jbdb.retrieve_song("", result_song);
   requireFalse(success, "retrieve_song must return false when empty string given for song file");

   // no songs in DB
   success = jbdb.retrieve_song("The-Who--Whos-Next--My-Wife.flac", result_song);
   requireFalse(success, "retrieve_song must return false when no songs exist");

   //TODO: implement test_retrieve_song
}

void TestJukeboxDB::test_insert_playlist() {
   TEST_CASE("test_insert_playlist");
   /*
   string test_dir = "/tmp/test_cpp_jukeboxdb_insert_playlist";
   FSTestCase test_case(*this, test_dir);
   string db_file = "jukebox_db.sqlite3";
   JukeboxDB jbdb(db_file);
   require(jbdb.open(), "open must return true");

   // normal insert
   string pl_uid = "70s-rock.json";
   string pl_name = "70s Rock";
   string pl_desc = "Favorite rock songs from the 70s";
   require(jbdb.insert_playlist(pl_uid, pl_name, pl_desc), "insert_playlist must return true");

   // 2nd insert with same data
   requireFalse(jbdb.insert_playlist(pl_uid, pl_name, pl_desc), "2nd insert attempt must return false");

   // insert with empty strings
   pl_uid = "";
   pl_name = "";
   pl_desc = "";
   requireFalse(jbdb.insert_playlist(pl_uid, pl_name, pl_desc), "insert attempt with empty strings must return false");
   */
}

void TestJukeboxDB::test_delete_playlist() {
   TEST_CASE("test_delete_playlist");
   /*
   string test_dir = "/tmp/test_cpp_jukeboxdb_delete_playlist";
   FSTestCase test_case(*this, test_dir);
   string db_file = "jukebox_db.sqlite3";
   JukeboxDB jbdb(db_file);
   require(jbdb.open(), "open must return true");

   string pl_name = "Rock-Jazz-Funk";

   // delete record that doesn't exist
   requireFalse(jbdb.delete_playlist(pl_name), "attempt to delete non-existing playlist must return false");

   // insert one
   require(jbdb.insert_playlist("Rock-Jazz-Funk", "Rock-Jazz-Funk"), "insert must return true");

   // delete with empty playlist name
   requireFalse(jbdb.delete_playlist(""), "delete playlist with empty string for name must return false");

   // normal delete
   require(jbdb.delete_playlist(pl_name), "normal delete must return true");

   // 2nd delete
   requireFalse(jbdb.delete_playlist(pl_name), "2nd delete attempt must return false");
   */
}

void TestJukeboxDB::test_insert_song() {
   TEST_CASE("test_insert_song");
   string test_dir = "/tmp/test_cpp_jukeboxdb_insert_song";
   FSTestCase test_case(*this, test_dir);
   string db_file = "jukebox_db.sqlite3";
   JukeboxDB jbdb(db_file);
   require(jbdb.open(), "open must return true");

   SongMetadata song;
   song.set_file_uid("The-Who--Whos-Next--My-Wife.flac");
   song.set_file_name("The-Who--Whos-Next--My-Wife.flac");
   song.set_origin_file_size(23827669L);
   song.set_stored_file_size(23827669L);
   song.set_pad_char_count(0L);
   song.set_file_time("2022-09-17 08:56:0.000");
   song.set_md5_hash("asdf");
   song.set_compressed(0);
   song.set_encrypted(0);
   song.set_container_name("w-artist-songs");
   song.set_object_name("The-Who--Whos-Next--My-Wife.flac");
   song.set_artist_uid("The-Who");
   song.set_artist_name("The Who");
   song.set_album_uid("Whos-Next");
   song.set_song_name("My Wife");

   require(jbdb.insert_song(song), "insert_song must return true");

   printf("=== expecting SQL error below\n");
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
   string test_dir = "/tmp/test_cpp_jukeboxdb_retrieve_album_songs";
   FSTestCase test_case(*this, test_dir);
   string db_file = "jukebox_db.sqlite3";
   JukeboxDB jbdb(db_file);
   require(jbdb.open(), "open must return true");

   insert_album(jbdb);

   vector<SongMetadata> songs;

   // empty string for artist
   songs.clear();
   jbdb.retrieve_album_songs("", "1984", songs);
   require(songs.empty(), "retrieve_album_songs must return empty list when artist name is missing");

   // empty string for both artist and album
   songs.clear();
   jbdb.retrieve_album_songs("", "", songs);
   require(!songs.empty(), "retrieve_album_songs must return non-empty list when artist name and album name are missing");

   // empty string for album
   songs.clear();
   jbdb.retrieve_album_songs("Van Halen", "", songs);
   require(songs.empty(), "retrieve_album_songs must return empty list when album name is missing");

   // non-matching artist and album
   songs.clear();
   jbdb.retrieve_album_songs("Led Zeppelin", "Houses of the Holy", songs);
   require(songs.empty(), "retrieve_album_songs must return empty list when no matching artist and album songs");

   // matching artist and album
   songs.clear();
   jbdb.retrieve_album_songs("Steely Dan", "Aja", songs);
   require(songs.size() == 7, "retrieve_album_songs must return 7 songs");
}

void TestJukeboxDB::test_songs_for_artist() {
   TEST_CASE("test_songs_for_artist");
   string test_dir = "/tmp/test_cpp_jukeboxdb_songs_for_artist";
   FSTestCase test_case(*this, test_dir);
   string db_file = "jukebox_db.sqlite3";
   JukeboxDB jbdb(db_file);
   require(jbdb.open(), "open must return true");

   vector<SongMetadata> songs;

   // empty string for artist
   jbdb.songs_for_artist("", songs);
   require(songs.empty(), "songs_for_artist must return empty list when artist name is missing");

   // no songs in DB
   jbdb.songs_for_artist("Van Halen", songs);
   require(songs.empty(), "songs_for_artist must return empty list when there are none");

   // no matching artist songs in DB
   SongMetadata song;
   song.set_file_uid("The-Who--Whos-Next--My-Wife.flac");
   song.set_file_name("The-Who--Whos-Next--My-Wife.flac");
   song.set_origin_file_size(23827669L);
   song.set_stored_file_size(23827669L);
   song.set_pad_char_count(0L);
   song.set_file_time("2022-09-17 08:56:0.000");
   song.set_md5_hash("asdf");
   song.set_compressed(0);
   song.set_encrypted(0);
   song.set_container_name("w-artist-songs");
   song.set_object_name("The-Who--Whos-Next--My-Wife.flac");
   song.set_artist_uid("The-Who");
   song.set_artist_name("The Who");
   song.set_album_uid("Whos-Next");
   song.set_song_name("My Wife");
   require(jbdb.insert_song(song), "insert_song must return true");

   jbdb.songs_for_artist("Van Halen", songs);
   require(songs.empty(), "songs_for_artist must return empty list when no matching artist songs");

   // add a matching artist song
   SongMetadata song2;
   song2.set_file_uid("Van-Halen--1984--Ill-Wait.flac");
   song2.set_file_name("Van-Halen--1984--Ill-Wait.flac");
   song2.set_origin_file_size(34648958L);
   song2.set_stored_file_size(34648958L);
   song2.set_pad_char_count(0L);
   song2.set_file_time("2022-09-18 08:05:0.000");
   song2.set_md5_hash("asdf");
   song2.set_compressed(0);
   song2.set_encrypted(0);
   song2.set_container_name("v-artist-songs");
   song2.set_object_name("Van-Halen--1984--Ill-Wait.flac");
   song2.set_artist_uid("Van-Halen");
   song2.set_artist_name("Van Halen");
   song2.set_album_uid("1984");
   song2.set_song_name("I'll Wait");
   require(jbdb.insert_song(song2), "insert_song must return true");

   jbdb.songs_for_artist("Van Halen", songs);
   require(songs.empty(), "songs_for_artist must return 1 song");

   //TODO: multiple matching and non-matching artist songs
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

   string song_uid = "The-Who--Whos-Next--My-Wife.flac";

   requireFalse(jbdb.delete_song(song_uid), "delete_song must return false for DB not open");
   require(jbdb.open(), "open must return true");

   requireFalse(jbdb.delete_song(""), "delete_song must return false for empty string song");
   requireFalse(jbdb.delete_song(song_uid), "delete_song must return false for empty DB");

   SongMetadata song;
   song.set_file_uid("The-Who--Whos-Next--My-Wife.flac");
   song.set_file_name("The-Who--Whos-Next--My-Wife.flac");
   song.set_origin_file_size(23827669L);
   song.set_stored_file_size(23827669L);
   song.set_pad_char_count(0L);
   song.set_file_time("2022-09-17 08:56:0.000");
   song.set_md5_hash("asdf");
   song.set_compressed(0);
   song.set_encrypted(0);
   song.set_container_name("w-artist-songs");
   song.set_object_name("The-Who--Whos-Next--My-Wife.flac");
   song.set_artist_uid("The-Who");
   song.set_artist_name("The Who");
   song.set_album_uid("Whos-Next");
   song.set_song_name("My Wife");

   require(jbdb.insert_song(song), "insert_song must return true");
   require(jbdb.delete_song(song_uid), "delete_song must return true for existing song");
   requireFalse(jbdb.delete_song(song_uid), "delete_song must return false on 2nd delete");

   jbdb.close();
}

