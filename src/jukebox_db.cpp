#include "jukebox_db.h"
#include "SQLiteDatabase.h"
#include "DBStatementArgs.h"
#include "DBInt.h"
#include "DBLong.h"
#include "DBString.h"
#include "jb_utils.h"

using namespace std;
using namespace chapeau;

JukeboxDB::JukeboxDB(const string& db_file_path, bool debug) :
   debug_print(debug),
   db_is_open(false),
   db_connection(NULL) {

   if (db_file_path.length() > 0) {
      metadata_db_file_path = db_file_path;
   } else {
      metadata_db_file_path = "jukebox_db.sqlite3";
   }
}

JukeboxDB::~JukeboxDB() {
   close();
}

bool JukeboxDB::is_open() const {
   return db_is_open;
}

bool JukeboxDB::open_db() {
   bool was_opened = false;
   db_connection = new SQLiteDatabase(metadata_db_file_path);
   if (db_connection->open()) {
      db_is_open = true;
      was_opened = true;
   }
   return was_opened;
}

bool JukeboxDB::open() {
   close();
   bool open_success = false;
   if (open_db()) {
      if (!have_tables()) {
         open_success = create_tables();
         if (!open_success) {
            if (debug_print) {
               printf("error: unable to create all tables\n");
            }
            db_connection->close();
            db_connection = NULL;
            db_is_open = false;
         }
      } else {
         open_success = true;
      }
   } else {
      if (debug_print) {
         printf("error: unable to open database\n");
      }
   }
   return open_success;
}

bool JukeboxDB::close() {
   bool did_close = false;
   if (db_connection != NULL) {
      db_connection->close();
      delete db_connection;
      db_connection = NULL;
      db_is_open = false;
      did_close = true;
   }
   return did_close;
}

bool JukeboxDB::create_table(const string& sql) {
   if (db_connection == NULL) {
      if (debug_print) {
         printf("cannot create table, DB is not open\n");
      }
      return false;
   }
   bool table_created = db_connection->executeUpdate(sql);
   if (!table_created) {
      if (debug_print) {
         printf("table create failed: %s\n", sql.c_str());
      }
   }
   return table_created;
}

bool JukeboxDB::create_tables() {
   if (db_is_open) {
      if (debug_print) {
         printf("creating tables\n");
      }

      string create_genre_table = "CREATE TABLE genre ("
                                     "genre_uid TEXT UNIQUE NOT NULL,"
                                     "genre_name TEXT UNIQUE NOT NULL,"
                                     "genre_description TEXT)";

      string create_artist_table = "CREATE TABLE artist ("
                                      "artist_uid TEXT UNIQUE NOT NULL,"
                                      "artist_name TEXT UNIQUE NOT NULL,"
                                      "artist_description TEXT)";

      string create_album_table = "CREATE TABLE album ("
                                     "album_uid TEXT UNIQUE NOT NULL,"
                                     "album_name TEXT UNIQUE NOT NULL,"
                                     "album_description TEXT,"
                                     "artist_uid TEXT NOT NULL REFERENCES artist(artist_uid),"
                                     "genre_uid TEXT REFERENCES genre(genre_uid))";

      string create_song_table = "CREATE TABLE song ("
                                    "song_uid TEXT UNIQUE NOT NULL,"
                                    "file_time TEXT,"
                                    "origin_file_size INTEGER,"
                                    "stored_file_size INTEGER,"
                                    "pad_char_count INTEGER,"
                                    "artist_name TEXT,"
                                    "artist_uid TEXT REFERENCES artist(artist_uid),"
                                    "song_name TEXT NOT NULL,"
                                    "md5_hash TEXT NOT NULL,"
                                    "compressed INTEGER,"
                                    "encrypted INTEGER,"
                                    "container_name TEXT NOT NULL,"
                                    "object_name TEXT NOT NULL,"
                                    "album_uid TEXT REFERENCES album(album_uid))";

      string create_playlist_table = "CREATE TABLE playlist ("
                                        "playlist_uid TEXT UNIQUE NOT NULL,"
                                        "playlist_name TEXT UNIQUE NOT NULL,"
                                        "playlist_description TEXT)";

      string create_playlist_song_table = "CREATE TABLE playlist_song ("
                                             "playlist_song_uid TEXT UNIQUE NOT NULL,"
                                             "playlist_uid TEXT NOT NULL REFERENCES playlist(playlist_uid),"
                                             "song_uid TEXT NOT NULL REFERENCES song(song_uid))";

      return create_table(create_genre_table) &&
             create_table(create_artist_table) &&
             create_table(create_album_table) &&
             create_table(create_song_table) &&
             create_table(create_playlist_table) &&
             create_table(create_playlist_song_table);
   } else {
      printf("create_tables: db_is_open is false\n");
      return false;
   }
}

bool JukeboxDB::have_tables() {
   bool have_tables_in_db = false;
   if (db_is_open && db_connection != NULL) {
      string sql = "SELECT name "
                   "FROM sqlite_master "
                   "WHERE type='table' AND name='song'";
      DBResultSet* rs = db_connection->executeQuery(sql);
      if (rs != NULL) {
         if (rs->next()) {
            string* name = rs->stringForColumnIndex(0);
            if (name != NULL) {
               if (name->length() > 0) {
                  have_tables_in_db = true;
               }
               delete name;
            }
         }
         delete rs;
      }
   }

   return have_tables_in_db;
}

string JukeboxDB::get_playlist(const string& playlist_name) {
   string pl_object;
   if (playlist_name.length() > 0 && db_connection != NULL) {
      string sql = "SELECT playlist_uid "
                   "FROM playlist "
                   "WHERE playlist_name = ?";
      DBStatementArgs args;
      args.add(new DBString(playlist_name));
      DBResultSet* rs = db_connection->executeQuery(sql, args);
      if (rs != NULL) {
         if (rs->next()) {
            string* playlist_uid = rs->stringForColumnIndex(0);
            if (playlist_uid != NULL) {
               pl_object = *playlist_uid;
               delete playlist_uid;
            }
         }
         delete rs;
      }
   }
   return pl_object;
}

bool JukeboxDB::songs_for_query(DBResultSet* rs,
                                vector<SongMetadata>& vec_songs) {
   int num_songs = 0;
   while (rs->next()) {
      SongMetadata song;
      string* file_uid = rs->stringForColumnIndex(0);
      if (file_uid != NULL) {
         song.fm.file_uid = *file_uid;
         delete file_uid;
      }
      string* file_time = rs->stringForColumnIndex(1);
      if (file_time != NULL) {
         song.fm.file_time = *file_time;
         delete file_time;
      }
      song.fm.origin_file_size = rs->longForColumnIndex(2);
      song.fm.stored_file_size = rs->longForColumnIndex(3);
      song.fm.pad_char_count = rs->longForColumnIndex(4);
      string* artist_name = rs->stringForColumnIndex(5);
      if (artist_name != NULL) {
         song.artist_name = *artist_name;
         delete artist_name;
      }
      string* artist_uid = rs->stringForColumnIndex(6);
      if (artist_uid != NULL) {
         song.artist_uid = *artist_uid;
         delete artist_uid;
      }
      string* song_name = rs->stringForColumnIndex(7);
      if (song_name != NULL) {
         song.song_name = *song_name;
         delete song_name;
      }
      string* md5_hash = rs->stringForColumnIndex(8);
      if (md5_hash != NULL) {
         song.fm.md5_hash = *md5_hash;
         delete md5_hash;
      }
      song.fm.compressed = rs->intForColumnIndex(9);
      song.fm.encrypted = rs->intForColumnIndex(10);
      string* container_name = rs->stringForColumnIndex(11);
      if (container_name != NULL) {
         song.fm.container_name = *container_name;
         delete container_name;
      }
      string* object_name = rs->stringForColumnIndex(12);
      if (object_name != NULL) {
         song.fm.object_name = *object_name;
         delete object_name;
      }
      string* album_uid = rs->stringForColumnIndex(13);
      if (album_uid != NULL) {
         song.album_uid = *album_uid;
         delete album_uid;
      } else {
         song.album_uid = "";
      }
      vec_songs.push_back(song);
      num_songs++;
   }
   return (num_songs > 0);
}

bool JukeboxDB::retrieve_song(const string& file_name, SongMetadata& song) {
   bool success = false;
   if (db_is_open) {
      string sql = "SELECT song_uid,"
                   "     file_time,"
                   "     origin_file_size,"
                   "     stored_file_size,"
                   "     pad_char_count,"
                   "     artist_name,"
                   "     artist_uid,"
                   "     song_name,"
                   "     md5_hash,"
                   "     compressed,"
                   "     encrypted,"
                   "     container_name,"
                   "     object_name,"
                   "     album_uid "
                   "FROM song "
                   "WHERE song_uid = ?";
      DBStatementArgs args;
      args.add(new DBString(file_name));
      DBResultSet* rs = db_connection->executeQuery(sql, args);
      if (rs != NULL) {
         vector<SongMetadata> song_results;
         if (songs_for_query(rs, song_results)) {
            delete rs;
            song = song_results[0];
            success = true;
         }
         delete rs;
      }
   }
   return success;
}

bool JukeboxDB::insert_playlist(const string& pl_uid,
                                const string& pl_name,
                                string pl_desc) {
   bool insert_success = false;

   if (db_is_open && pl_uid.length() > 0 && pl_name.length() > 0) {
      string sql = "INSERT INTO playlist VALUES (?,?,?)";
      DBStatementArgs args;
      args.add(new DBString(pl_uid));
      args.add(new DBString(pl_name));
      args.add(new DBString(pl_desc));
      if (db_connection->executeUpdate(sql, args)) {
         insert_success = true;
      } else {
         printf("error inserting playlist\n");
      }
   }

   return insert_success;
}

bool JukeboxDB::delete_playlist(const string& pl_name) {
   bool delete_success = false;

   if (db_is_open && pl_name.length() > 0) {
      string sql = "DELETE "
                   "FROM playlist "
                   "WHERE playlist_name = ?";
      DBStatementArgs args;
      args.add(new DBString(pl_name));
      delete_success = db_connection->executeUpdate(sql, args);
   }

   return delete_success;
}

bool JukeboxDB::insert_song(const SongMetadata& song) {
   bool insert_success = false;

   if (db_is_open) {
      string sql = "INSERT INTO song "
                   "VALUES (?,"
                           "?,"
                           "?,"
                           "?,"
                           "?,"
                           "?,"
                           "?,"
                           "?,"
                           "?,"
                           "?,"
                           "?,"
                           "?,"
                           "?,"
                           "?)";

      DBStatementArgs args;
      args.add(new DBString(song.fm.file_uid));
      args.add(new DBString(song.fm.file_time));
      args.add(new DBLong(song.fm.origin_file_size));
      args.add(new DBLong(song.fm.stored_file_size));
      args.add(new DBLong(song.fm.pad_char_count));
      args.add(new DBString(song.artist_name));
      args.add(new DBString(song.artist_uid));
      args.add(new DBString(song.song_name));
      args.add(new DBString(song.fm.md5_hash));
      args.add(new DBInt(song.fm.compressed));
      args.add(new DBInt(song.fm.encrypted));
      args.add(new DBString(song.fm.container_name));
      args.add(new DBString(song.fm.object_name));
      args.add(new DBString(song.album_uid));

      bool success = db_connection->executeUpdate(sql, args);
      if (success) {
         insert_success = true;
      } else {
         printf("error inserting song\n");
      }
   }

   return insert_success;
}

bool JukeboxDB::update_song(const SongMetadata& song) {
   bool update_success = false;

   if (db_is_open && song.fm.file_uid.length() > 0) {
      string sql = "UPDATE song "
                   "SET file_time = ?,"
                       "origin_file_size = ?,"
                       "stored_file_size = ?,"
                       "pad_char_count = ?,"
                       "artist_name = ?,"
                       "artist_uid = ?,"
                       "song_name = ?,"
                       "md5_hash = ?,"
                       "compressed = ?,"
                       "encrypted = ?,"
                       "container_name = ?,"
                       "object_name = ?,"
                       "album_uid = ? "
                   "WHERE song_uid = ?";
      DBStatementArgs args;
      args.add(new DBString(song.fm.file_time));
      args.add(new DBLong(song.fm.origin_file_size));
      args.add(new DBLong(song.fm.stored_file_size));
      args.add(new DBLong(song.fm.pad_char_count));
      args.add(new DBString(song.artist_name));
      args.add(new DBString(""));
      args.add(new DBString(song.song_name));
      args.add(new DBString(song.fm.md5_hash));
      args.add(new DBInt(song.fm.compressed));
      args.add(new DBInt(song.fm.encrypted));
      args.add(new DBString(song.fm.container_name));
      args.add(new DBString(song.fm.object_name));
      args.add(new DBString(song.album_uid));
      args.add(new DBString(song.fm.file_uid));

      bool success = db_connection->executeUpdate(sql, args);
      if (success) {
         update_success = true;
      } else {
         printf("error updating song\n");
      }
   }

   return update_success;
}

bool JukeboxDB::store_song_metadata(const SongMetadata& song) {
   SongMetadata db_song;
   if (retrieve_song(song.fm.file_uid, db_song)) {
      bool is_same_song = (song == db_song);
      if (!is_same_song) {
         return update_song(song);
      } else {
         return true;  // no insert or update needed (already up-to-date)
      }
   } else {
      // song is not in the database, insert it
      return insert_song(song);
   }
}

string JukeboxDB::sql_where_clause(bool using_encryption,
                                   bool using_compression) {
   string encryption;
   if (using_encryption) {
      encryption = "1";
   } else {
      encryption = "0";
   }

   string compression;
   if (using_compression) {
      compression = "1";
   } else {
      compression = "0";
   }

   string where_clause = "";
   where_clause += " WHERE ";
   where_clause += "encrypted = ";
   where_clause += encryption;
   where_clause += " AND ";
   where_clause += "compressed = ";
   where_clause += compression;
   return where_clause;
}

vector<SongMetadata> JukeboxDB::retrieve_album_songs(const string& artist,
                                                     const string& album) {
   vector<SongMetadata> songs;
   if (db_is_open) {
      string sql = "SELECT song_uid,"
                          "file_time,"
                          "origin_file_size,"
                          "stored_file_size,"
                          "pad_char_count,"
                          "artist_name,"
                          "artist_uid,"
                          "song_name,"
                          "md5_hash,"
                          "compressed,"
                          "encrypted,"
                          "container_name,"
                          "object_name,"
                          "album_uid "
                   "FROM song";
      sql += sql_where_clause();
      //if len(artist) > 0:
      //    sql += " AND artist_name='%s'" % artist
      if (album.length() > 0) {
          string encoded_artist = JBUtils::encode_value(artist);
          string encoded_album = JBUtils::encode_value(album);
          sql += " AND object_name LIKE '{encoded_artist}--{encoded_album}%'";
      }
      DBResultSet* rs = db_connection->executeQuery(sql);
      if (rs != NULL) {
         songs_for_query(rs, songs);
         delete rs;
      }
   }
   return songs;
}

vector<SongMetadata> JukeboxDB::songs_for_artist(const string& artist_name) {
   vector<SongMetadata> songs;
   if (db_is_open) {
      string sql = "SELECT song_uid,"
                          "file_time,"
                          "origin_file_size,"
                          "stored_file_size,"
                          "pad_char_count,"
                          "artist_name,"
                          "artist_uid,"
                          "song_name,"
                          "md5_hash,"
                          "compressed,"
                          "encrypted,"
                          "container_name,"
                          "object_name,"
                          "album_uid "
                   "FROM song";
      sql += sql_where_clause();
      sql += " AND artist = ?";
      DBStatementArgs args;
      args.add(new DBString(artist_name));
      DBResultSet* rs = db_connection->executeQuery(sql, args);
      if (rs != NULL) {
         songs_for_query(rs, songs);
         delete rs;
      }
   }
   return songs;
}

void JukeboxDB::show_listings() {
   if (db_is_open) {
      string sql = "SELECT artist_name, song_name "
                   "FROM song "
                   "ORDER BY artist_name, song_name";
      DBResultSet* rs = db_connection->executeQuery(sql);
      if (rs != NULL) {
         while (rs->next()) {
            string* artist_name = rs->stringForColumnIndex(0);
            string* song_name = rs->stringForColumnIndex(1);
            if (artist_name != NULL && song_name != NULL) {
               printf("%s, %s\n", artist_name->c_str(), song_name->c_str());
            }
            delete artist_name;
            delete song_name;
         }
         delete rs;
      }
   }
}

void JukeboxDB::show_artists() {
   if (db_is_open) {
      string sql = "SELECT DISTINCT artist_name "
                   "FROM song "
                   "ORDER BY artist_name";
      DBResultSet* rs = db_connection->executeQuery(sql);
      if (rs != NULL) {
         while (rs->next()) {
            string* artist_name = rs->stringForColumnIndex(0);
            if (artist_name != NULL) {
               printf("%s\n", artist_name->c_str());
               delete artist_name;
            }
         }
         delete rs;
      }
   }
}

void JukeboxDB::show_genres() {
   if (db_is_open) {
      string sql = "SELECT genre_name "
                   "FROM genre "
                   "ORDER BY genre_name";
      DBResultSet* rs = db_connection->executeQuery(sql);
      if (rs != NULL) {
         while (rs->next()) {
            string* genre_name = rs->stringForColumnIndex(0);
            if (genre_name != NULL) {
               printf("%s\n", genre_name->c_str());
               delete genre_name;
            }
         }
         delete rs;
      }
   }
}

void JukeboxDB::show_artist_albums(const string& artist_name) {
   if (db_is_open) {
      string sql = "SELECT b.album_name "
                   "FROM artist a, album b "
                   "WHERE a.artist_uid = b.artist_uid "
                   "AND a.artist_name = ?";
      DBStatementArgs args;
      args.add(new DBString(artist_name));
      DBResultSet* rs = db_connection->executeQuery(sql, args);
      if (rs != NULL) {
         while (rs->next()) {
            string* album_name = rs->stringForColumnIndex(0);
            if (album_name != NULL) {
               printf("%s\n", album_name->c_str());
               delete album_name;
            }
         }
         delete rs;
      }
   } else {
      printf("error: DB is not open\n");
   }
}

void JukeboxDB::show_albums() {
   if (db_is_open) {
      string sql = "SELECT album.album_name, artist.artist_name "
                   "FROM album, artist "
                   "WHERE album.artist_uid = artist.artist_uid "
                   "ORDER BY album.album_name";
      DBResultSet* rs = db_connection->executeQuery(sql);
      if (rs != NULL) {
         while (rs->next()) {
            string* album_name = rs->stringForColumnIndex(0);
            string* artist_name = rs->stringForColumnIndex(1);
            if (album_name != NULL && artist_name != NULL) {
               printf("%s (%s)\n", album_name->c_str(), artist_name->c_str());
            }
            delete album_name;
            delete artist_name;
         }
         delete rs;
      }
   }
}

void JukeboxDB::show_playlists() {
   if (db_is_open) {
      string sql = "SELECT playlist_uid, playlist_name "
                   "FROM playlist "
                   "ORDER BY playlist_uid";
      DBResultSet* rs = db_connection->executeQuery(sql);
      if (rs != NULL) {
         while (rs->next()) {
            string* playlist_uid = rs->stringForColumnIndex(0);
            string* playlist_name = rs->stringForColumnIndex(1);
            if (playlist_uid != NULL && playlist_name != NULL) {
               printf("%s - %s\n", playlist_uid->c_str(), playlist_name->c_str());
            }
            delete playlist_uid;
            delete playlist_name;
         }
         delete rs;
      }
   }
}

bool JukeboxDB::delete_song(const string& song_uid) {
   bool was_deleted = false;
   if (db_is_open) {
      if (song_uid.length() > 0) {
         string sql = "DELETE FROM song WHERE song_uid = ?";
         DBStatementArgs args;
         args.add(new DBString(song_uid));
         was_deleted = db_connection->executeUpdate(sql, args);
         if (!was_deleted) {
            printf("error deleting song %s\n", song_uid.c_str());
         }
      }
   } 

   return was_deleted;
}

