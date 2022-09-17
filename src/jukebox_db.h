#ifndef JUKEBOX_DB_H
#define JUKEBOX_DB_H

#include <string>
#include <vector>

#include "Database.h"
#include "song_metadata.h"


class JukeboxDB {
private:
   bool debug_print;
   bool db_is_open;
   chapeau::Database* db_connection;
   std::string metadata_db_file_path;

public:
   JukeboxDB(const std::string& metadata_db_file_path,
             bool debug_print=true);

   bool is_open() const;
   bool open();
   bool open_db();
   bool close();

   bool create_table(const std::string& sql);
   bool create_tables();

   bool have_tables();

   std::string get_playlist(const std::string& playlist_name);

   bool songs_for_query(chapeau::DBResultSet* rs,
                        std::vector<SongMetadata*>& vec_songs);

   SongMetadata* retrieve_song(const std::string& file_name);
   bool insert_playlist(const std::string& pl_uid,
                        const std::string& pl_name,
			std::string pl_desc = "");
   bool delete_playlist(const std::string& pl_name);
   bool insert_song(const SongMetadata& song);
   bool update_song(const SongMetadata& song);
   bool store_song_metadata(const SongMetadata& song);
   std::string sql_where_clause(bool using_encryption = false,
                                bool using_compression = false);
   std::vector<SongMetadata*> retrieve_album_songs(const std::string& artist,
                                                   const std::string& album);
   std::vector<SongMetadata*> songs_for_artist(const std::string& artist_name);
   void show_listings();
   void show_artists();
   void show_genres();
   void show_artist_albums(const std::string& artist_name);
   void show_albums();
   void show_playlists();
   bool delete_song(const std::string& song_uid);
};

#endif

