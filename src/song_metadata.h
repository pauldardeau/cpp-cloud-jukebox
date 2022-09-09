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

   void from_dictionary(const std::map<std::string, PropertyValue*>& dictionary,
                        std::string prefix) {
      fm.from_dictionary(dictionary, prefix);

      std::string key = prefix + "artist_uid";
      const auto it_end = dictionary.end();
      auto it = dictionary.find(key);
      if (it != it_end) {
         const PropertyValue* pv = it->second;
	 if (pv->is_string()) {
            artist_uid = pv->get_string_value();
	 }
      }

      key = prefix + "artist_name";
      it = dictionary.find(key);
      if (it != it_end) {
         const PropertyValue* pv = it->second;
         if (pv->is_string()) {
            artist_name = pv->get_string_value();
         }
      }

      key = prefix + "album_uid";
      it = dictionary.find(key);
      if (it != it_end) {
         const PropertyValue* pv = it->second;
         if (pv->is_string()) {
            album_uid = pv->get_string_value();
         }
      }

      key = prefix + "song_name";
      it = dictionary.find(key);
      if (it != it_end) {
         const PropertyValue* pv = it->second;
         if (pv->is_string()) {
            song_name = pv->get_string_value();
         }
      }
   }

   std::map<std::string, PropertyValue*> to_dictionary(std::string prefix="") const {
      std::map<std::string, PropertyValue*> d;
      //d[prefix + "fm"] = fm.to_dictionary(prefix)},
      d[prefix + "artist_uid"] = new StrPropertyValue(artist_uid);
      d[prefix + "artist_name"] = new StrPropertyValue(artist_name);
      d[prefix + "album_uid"] = new StrPropertyValue(album_uid);
      d[prefix + "song_name"] = new StrPropertyValue(song_name);
      return d;
   }
};

#endif

