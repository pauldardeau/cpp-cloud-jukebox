#ifndef SONG_METADATA_H
#define SONG_METADATA_H

#include <string>
#include <map>
#include "file_metadata.h"
#include "data_types.h"


class SongMetadata {
private:
   FileMetadata m_fm;
   std::string m_artist_uid;
   std::string m_artist_name;
   std::string m_album_uid;
   std::string m_song_name;


public:
   SongMetadata() {
   }

   SongMetadata(const SongMetadata& copy) :
      m_fm(copy.m_fm),
      m_artist_uid(copy.m_artist_uid),
      m_artist_name(copy.m_artist_name),
      m_album_uid(copy.m_album_uid),
      m_song_name(copy.m_song_name) {
   }

   ~SongMetadata() {}

   SongMetadata& operator=(const SongMetadata& copy) {
      if (this == &copy) {
         return *this;
      }

      m_fm = copy.m_fm;
      m_artist_uid = copy.m_artist_uid;
      m_artist_name = copy.m_artist_name;
      m_album_uid = copy.m_album_uid;
      m_song_name = copy.m_song_name;

      return *this;
   }

   bool operator==(const SongMetadata& other) const {
      if (this == &other) {
         return true;
      }

      return m_fm == other.m_fm &&
             m_artist_uid == other.m_artist_uid &&
             m_artist_name == other.m_artist_name &&
             m_album_uid == other.m_album_uid &&
             m_song_name == other.m_song_name;
   }

   bool operator!=(const SongMetadata& other) const {
      return !(this->operator==(other));
   }

   void from_dictionary(const PropertySet& dictionary,
                        std::string prefix) {
      m_fm.from_dictionary(dictionary, prefix);

      std::string key = prefix + "artist_uid";
      const PropertyValue* pv = dictionary.get(key);
      if (pv != nullptr && pv->is_string()) {
         m_artist_uid = pv->get_string_value();
      }

      key = prefix + "artist_name";
      pv = dictionary.get(key);
      if (pv != nullptr && pv->is_string()) {
         m_artist_name = pv->get_string_value();
      }

      key = prefix + "album_uid";
      pv = dictionary.get(key);
      if (pv != nullptr && pv->is_string()) {
         m_album_uid = pv->get_string_value();
      }

      key = prefix + "song_name";
      pv = dictionary.get(key);
      if (pv != nullptr && pv->is_string()) {
         m_song_name = pv->get_string_value();
      }
   }

   void to_dictionary(PropertySet& d, std::string prefix="") const {
      m_fm.to_dictionary(d, prefix);
      d.add(prefix + "artist_uid", new StrPropertyValue(m_artist_uid));
      d.add(prefix + "artist_name", new StrPropertyValue(m_artist_name));
      d.add(prefix + "album_uid", new StrPropertyValue(m_album_uid));
      d.add(prefix + "song_name", new StrPropertyValue(m_song_name));
   }

   const FileMetadata& get_file_metadata() const {
      return m_fm;
   }

   const std::string& get_artist_uid() const {
      return m_artist_uid;
   }

   const std::string& get_artist_name() const {
      return m_artist_name;
   }

   const std::string& get_album_uid() const {
      return m_album_uid;
   }

   const std::string& get_song_name() const {
      return m_song_name;
   }

   const std::string& get_object_name() const {
      return m_fm.get_object_name();
   }

   const std::string& get_container_name() const {
      return m_fm.get_container_name();
   }

   const std::string& get_file_uid() const {
      return m_fm.get_file_uid();
   }

   const std::string& get_md5_hash() const {
      return m_fm.get_md5_hash();
   }

   const std::string& get_file_time() const {
      return m_fm.get_file_time();
   }

   unsigned long get_stored_file_size() const {
      return m_fm.get_stored_file_size();
   }

   unsigned long get_origin_file_size() const {
      return m_fm.get_origin_file_size();
   }

   unsigned long get_pad_char_count() const {
      return m_fm.get_pad_char_count();
   }

   int get_compressed() const {
      return m_fm.get_compressed();
   }

   int get_encrypted() const {
      return m_fm.get_encrypted();
   }

   void set_artist_uid(const std::string& s) {
      m_artist_uid = s;
   }

   void set_artist_name(const std::string& s) {
      m_artist_name = s;
   }

   void set_album_uid(const std::string& s) {
      m_album_uid = s;
   }

   void set_song_name(const std::string& s) {
      m_song_name = s;
   }

   void set_container_name(const std::string& s) {
      m_fm.set_container_name(s);
   }

   void set_object_name(const std::string& s) {
      m_fm.set_object_name(s);
   }

   void set_file_uid(const std::string& s) {
      m_fm.set_file_uid(s);
   }

   void set_file_time(const std::string& s) {
      m_fm.set_file_time(s);
   }

   void set_md5_hash(const std::string& s) {
      m_fm.set_md5_hash(s);
   }

   void set_compressed(int i) {
      m_fm.set_compressed(i);
   }

   void set_encrypted(int i) {
      m_fm.set_encrypted(i);
   }

   void set_pad_char_count(unsigned long count) {
      m_fm.set_pad_char_count(count);
   }

   void set_stored_file_size(unsigned long file_size) {
      m_fm.set_stored_file_size(file_size);
   }

   void set_origin_file_size(unsigned long file_size) {
      m_fm.set_origin_file_size(file_size);
   }

};

#endif

