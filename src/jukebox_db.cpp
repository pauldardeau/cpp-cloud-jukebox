#include <string.h>
#include "jukebox_db.h"
#include "SQLiteDatabase.h"
#include "DBStatementArgs.h"
#include "DBInt.h"
#include "DBLong.h"
#include "DBString.h"
#include "jb_utils.h"

using namespace std;
using namespace chapeau;

//*****************************************************************************

JukeboxDB::JukeboxDB(const string& db_file_path, bool debug) :
   m_debug_print(debug),
   m_db_is_open(false) {

   if (!db_file_path.empty()) {
      m_metadata_db_file_path = db_file_path;
   } else {
      m_metadata_db_file_path = "jukebox_db.sqlite3";
   }
}

//*****************************************************************************

JukeboxDB::~JukeboxDB() {
   close();
}

//*****************************************************************************

bool JukeboxDB::is_open() const {
   return m_db_is_open;
}

//*****************************************************************************

bool JukeboxDB::open_db() {
   bool was_opened = false;
   m_db_connection.reset(new SQLiteDatabase(m_metadata_db_file_path));
   if (m_db_connection->open()) {
      m_db_is_open = true;
      was_opened = true;
   }
   return was_opened;
}

//*****************************************************************************

bool JukeboxDB::open() {
   close();
   bool open_success = false;
   if (open_db()) {
      if (!have_tables()) {
         open_success = create_tables();
         if (!open_success) {
            if (m_debug_print) {
               printf("error: unable to create all tables\n");
            }
            m_db_connection->close();
            m_db_connection.reset();
            m_db_is_open = false;
         }
      } else {
         open_success = true;
      }
   } else {
      if (m_debug_print) {
         printf("error: unable to open database\n");
      }
   }
   return open_success;
}

//*****************************************************************************

bool JukeboxDB::close() {
   bool did_close = false;
   if (m_db_connection) {
      m_db_connection->close();
      m_db_connection.reset();
      m_db_is_open = false;
      did_close = true;
   }
   return did_close;
}

//*****************************************************************************

bool JukeboxDB::create_table(const string& sql) {
   if (!m_db_connection) {
      if (m_debug_print) {
         printf("cannot create table, DB is not open\n");
      }
      return false;
   }
   unsigned long rowsAffectedCount = 0L;
   bool table_created = m_db_connection->executeUpdate(sql, rowsAffectedCount);
   if (!table_created) {
      if (m_debug_print) {
         printf("table create failed: %s\n", sql.c_str());
      }
   }
   return table_created;
}

//*****************************************************************************

bool JukeboxDB::create_tables() {
   if (m_db_is_open) {
      if (m_debug_print) {
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

      return create_table(create_genre_table) &&
             create_table(create_artist_table) &&
             create_table(create_album_table) &&
             create_table(create_song_table);
   } else {
      printf("create_tables: db_is_open is false\n");
      return false;
   }
}

//*****************************************************************************

bool JukeboxDB::have_tables() {
   bool have_tables_in_db = false;
   if (m_db_is_open && m_db_connection) {
      string sql = "SELECT name "
                   "FROM sqlite_master "
                   "WHERE type='table' AND name='song'";
      unique_ptr<DBResultSet> rs(m_db_connection->executeQuery(sql));
      if (rs) {
         if (rs->next()) {
            string name;
            rs->stringForColumnIndex(0, name);
            if (!name.empty()) {
               have_tables_in_db = true;
            }
         }
      }
   }

   return have_tables_in_db;
}

//*****************************************************************************

bool JukeboxDB::songs_for_query(DBResultSet* rs,
                                vector<SongMetadata>& vec_songs) {
   int num_songs = 0;
   while (rs->next()) {
      SongMetadata song;

      string file_uid;
      if (rs->stringForColumnIndex(0, file_uid)) {
         song.fm.file_uid = file_uid;
      }

      string file_time;
      if (rs->stringForColumnIndex(1, file_time)) {
         song.fm.file_time = file_time;
      }

      song.fm.origin_file_size = rs->longForColumnIndex(2);
      song.fm.stored_file_size = rs->longForColumnIndex(3);
      song.fm.pad_char_count = rs->longForColumnIndex(4);

      string artist_name;
      if (rs->stringForColumnIndex(5, artist_name)) {
         song.artist_name = artist_name;
      }

      string artist_uid;
      if (rs->stringForColumnIndex(6, artist_uid)) {
         song.artist_uid = artist_uid;
      }

      string song_name;
      if (rs->stringForColumnIndex(7, song_name)) {
         song.song_name = song_name;
      }

      string md5_hash;
      if (rs->stringForColumnIndex(8, md5_hash)) {
         song.fm.md5_hash = md5_hash;
      }

      song.fm.compressed = rs->intForColumnIndex(9);
      song.fm.encrypted = rs->intForColumnIndex(10);

      string container_name;
      if (rs->stringForColumnIndex(11, container_name)) {
         song.fm.container_name = container_name;
      }

      string object_name;
      if (rs->stringForColumnIndex(12, object_name)) {
         song.fm.object_name = object_name;
      }

      string album_uid;
      if (rs->stringForColumnIndex(13, album_uid)) {
         song.album_uid = album_uid;
      } else {
         song.album_uid = "";
      }

      vec_songs.push_back(song);
      num_songs++;
   }
   return (num_songs > 0);
}

//*****************************************************************************

bool JukeboxDB::retrieve_song(const string& file_name, SongMetadata& song) {
   bool success = false;
   if (m_db_is_open) {
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
      unique_ptr<DBResultSet> rs(m_db_connection->executeQuery(sql, args));
      if (rs) {
         vector<SongMetadata> song_results;
         if (songs_for_query(rs.get(), song_results)) {
            if (song_results.size() > 0) {
               song = song_results[0];
               success = true;
            }
         }
      }
   }

   return success;
}

//*****************************************************************************

bool JukeboxDB::insert_song(const SongMetadata& song) {
   bool insert_success = false;

   if (m_db_is_open) {
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

      unsigned long rowsAffectedCount = 0L;
      bool success = m_db_connection->executeUpdate(sql, args, rowsAffectedCount);
      if (success) {
         if (rowsAffectedCount == 1L) {
            insert_success = true;
         }
      } else {
         printf("error inserting song\n");
      }
   }

   return insert_success;
}

//*****************************************************************************

bool JukeboxDB::update_song(const SongMetadata& song) {
   bool update_success = false;

   if (m_db_is_open && !song.fm.file_uid.empty()) {
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

      unsigned long rowsAffectedCount = 0L;
      bool success = m_db_connection->executeUpdate(sql, args, rowsAffectedCount);
      if (success) {
         if (rowsAffectedCount == 1L) {
            update_success = true;
         }
      } else {
         printf("error updating song\n");
      }
   }

   return update_success;
}

//*****************************************************************************

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

//*****************************************************************************

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

//*****************************************************************************

vector<SongMetadata> JukeboxDB::retrieve_album_songs(const string& artist,
                                                     const string& album) {
   vector<SongMetadata> songs;
   if (m_db_is_open) {
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
      if (!artist.empty()) {
          string encoded_artist = JBUtils::encode_value(artist);
          char like_clause[4096];
          memset(like_clause, 0, sizeof(like_clause));
          if (!album.empty()) {
              string encoded_album = JBUtils::encode_value(album);
              snprintf(like_clause, 4096, " AND object_name LIKE '%s--%s%%'", encoded_artist.c_str(), encoded_album.c_str());
          } else {
              snprintf(like_clause, 4096, " AND object_name LIKE '%s--%%'", encoded_artist.c_str());
          }
          sql += string(like_clause);
      }
      
      unique_ptr<DBResultSet> rs(m_db_connection->executeQuery(sql));
      if (rs) {
         songs_for_query(rs.get(), songs);
      }
   }

   return songs;
}

//*****************************************************************************

vector<SongMetadata> JukeboxDB::songs_for_artist(const string& artist_name) {
   vector<SongMetadata> songs;
   if (m_db_is_open) {
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
      unique_ptr<DBResultSet> rs(m_db_connection->executeQuery(sql, args));
      if (rs) {
         songs_for_query(rs.get(), songs);
      }
   }

   return songs;
}

//*****************************************************************************

void JukeboxDB::show_listings() {
   if (m_db_is_open) {
      string sql = "SELECT artist_name, song_name "
                   "FROM song "
                   "ORDER BY artist_name, song_name";
      unique_ptr<DBResultSet> rs(m_db_connection->executeQuery(sql));
      if (rs) {
         while (rs->next()) {
            string artist_name;
            string song_name;
            rs->stringForColumnIndex(0, artist_name);
            rs->stringForColumnIndex(1, song_name);
            if (!artist_name.empty() && !song_name.empty()) {
               printf("%s, %s\n", artist_name.c_str(), song_name.c_str());
            }
         }
      }
   }
}

//*****************************************************************************

void JukeboxDB::show_artists() {
   if (m_db_is_open) {
      string sql = "SELECT DISTINCT artist_name "
                   "FROM song "
                   "ORDER BY artist_name";
      unique_ptr<DBResultSet> rs(m_db_connection->executeQuery(sql));
      if (rs) {
         while (rs->next()) {
            string artist_name;
            rs->stringForColumnIndex(0, artist_name);
            if (!artist_name.empty()) {
               printf("%s\n", artist_name.c_str());
            }
         }
      }
   }
}

//*****************************************************************************

void JukeboxDB::show_genres() {
   if (m_db_is_open) {
      string sql = "SELECT genre_name "
                   "FROM genre "
                   "ORDER BY genre_name";
      unique_ptr<DBResultSet> rs(m_db_connection->executeQuery(sql));
      if (rs) {
         while (rs->next()) {
            string genre_name;
            rs->stringForColumnIndex(0, genre_name);
            if (!genre_name.empty()) {
               printf("%s\n", genre_name.c_str());
            }
         }
      }
   }
}

//*****************************************************************************

void JukeboxDB::show_artist_albums(const string& artist_name) {
   if (m_db_is_open) {
      string sql = "SELECT b.album_name "
                   "FROM artist a, album b "
                   "WHERE a.artist_uid = b.artist_uid "
                   "AND a.artist_name = ?";
      DBStatementArgs args;
      args.add(new DBString(artist_name));
      unique_ptr<DBResultSet> rs(m_db_connection->executeQuery(sql, args));
      if (rs) {
         while (rs->next()) {
            string album_name;
            rs->stringForColumnIndex(0, album_name);
            if (!album_name.empty()) {
               printf("%s\n", album_name.c_str());
            }
         }
      }
   } else {
      printf("error: DB is not open\n");
   }
}

//*****************************************************************************

void JukeboxDB::show_albums() {
   if (m_db_is_open) {
      string sql = "SELECT album.album_name, artist.artist_name "
                   "FROM album, artist "
                   "WHERE album.artist_uid = artist.artist_uid "
                   "ORDER BY album.album_name";
      unique_ptr<DBResultSet> rs(m_db_connection->executeQuery(sql));
      if (rs) {
         while (rs->next()) {
            string album_name;
            string artist_name;
            rs->stringForColumnIndex(0, album_name);
            rs->stringForColumnIndex(1, artist_name);
            if (!album_name.empty() && !artist_name.empty()) {
               printf("%s (%s)\n", album_name.c_str(), artist_name.c_str());
            }
         }
      }
   }
}

//*****************************************************************************

bool JukeboxDB::delete_song(const string& song_uid) {
   bool was_deleted = false;
   if (m_db_is_open) {
      if (!song_uid.empty()) {
         string sql = "DELETE FROM song WHERE song_uid = ?";
         DBStatementArgs args;
         args.add(new DBString(song_uid));
         unsigned long rowsAffectedCount = 0L;
         bool sql_success = m_db_connection->executeUpdate(sql, args, rowsAffectedCount);
         if (sql_success) {
            if (rowsAffectedCount == 1L) {
               was_deleted = true;
            }
         } else {
            printf("error deleting song %s\n", song_uid.c_str());
         }
      }
   }

   return was_deleted;
}

//*****************************************************************************

