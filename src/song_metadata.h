#ifndef SONG_METADATA_H
#define SONG_METADATA_H

#include <string>
#include <map>
#include "file_metadata.h"
#include "data_types.h"


class SongMetadata {
public:
   FileMetadata fm;
   std::string artist_uid;
   std::string artist_name;
   std::string album_uid;
   std::string song_name;

   SongMetadata() {
   }

   SongMetadata(const SongMetadata& copy) :
      fm(copy.fm),
      artist_uid(copy.artist_uid),
      artist_name(copy.artist_name),
      album_uid(copy.album_uid),
      song_name(copy.song_name) {
   }

   ~SongMetadata() {}

   SongMetadata& operator=(const SongMetadata& copy) {
      if (this == &copy) {
         return *this;
      }

      fm = copy.fm;
      artist_uid = copy.artist_uid;
      artist_name = copy.artist_name;
      album_uid = copy.album_uid;
      song_name = copy.song_name;

      return *this;
   }

   bool operator==(const SongMetadata& other) const {
      if (this == &other) {
         return true;
      }

      return fm == other.fm &&
             artist_uid == other.artist_uid &&
             artist_name == other.artist_name &&
             album_uid == other.album_uid &&
             song_name == other.song_name;
   }

   bool operator!=(const SongMetadata& other) const {
      return !(this->operator==(other));
   }

   void from_dictionary(const PropertySet& dictionary,
                        std::string prefix) {
      fm.from_dictionary(dictionary, prefix);

      std::string key = prefix + "artist_uid";
      const PropertyValue* pv = dictionary.get(key);
      if (pv != NULL && pv->is_string()) {
         artist_uid = pv->get_string_value();
      }

      key = prefix + "artist_name";
      pv = dictionary.get(key);
      if (pv != NULL && pv->is_string()) {
         artist_name = pv->get_string_value();
      }

      key = prefix + "album_uid";
      pv = dictionary.get(key);
      if (pv != NULL && pv->is_string()) {
         album_uid = pv->get_string_value();
      }

      key = prefix + "song_name";
      pv = dictionary.get(key);
      if (pv != NULL && pv->is_string()) {
         song_name = pv->get_string_value();
      }
   }

   void to_dictionary(PropertySet& d, std::string prefix="") const {
      fm.to_dictionary(d, prefix);
      d.add(prefix + "artist_uid", new StrPropertyValue(artist_uid));
      d.add(prefix + "artist_name", new StrPropertyValue(artist_name));
      d.add(prefix + "album_uid", new StrPropertyValue(album_uid));
      d.add(prefix + "song_name", new StrPropertyValue(song_name));
   }
};

#endif

