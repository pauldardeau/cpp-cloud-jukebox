#include <string.h>
#include "jb_utils.h"
#include "StrUtils.h"

using namespace std;

static const string DOUBLE_DASHES = "--";

string JBUtils::unencode_value(const string& encoded_value) {
   string new_value = encoded_value;
   chaudiere::StrUtils::replaceAll(new_value, "-", " ");
   return new_value;
}

string JBUtils::encode_value(const string& value) {
   string new_value = value;
   chaudiere::StrUtils::replaceAll(new_value, " ", "-");
   return new_value;
}

string JBUtils::encode_artist_album(const string& artist,
                                    const string& album) {
   return encode_value(artist) + DOUBLE_DASHES +
          encode_value(album);
}

string JBUtils::encode_artist_album_song(const string& artist,
                                         const string& album,
                                         const string& song) {
   return encode_artist_album(artist, album) + DOUBLE_DASHES +
          encode_value(song);
}

string JBUtils::remove_punctuation(const string& s) {
   size_t s_len = s.length();
   size_t pos_punct = strcspn(s.c_str(), "'!?");
   if (pos_punct < s_len) {
      bool removing_punctuation = true;
      string updated = s;
      while (removing_punctuation) {
         pos_punct = strcspn(updated.c_str(), "'!?");
         if (pos_punct < updated.length()) {
            string before = updated.substr(0, pos_punct);
	    string after = updated.substr(pos_punct+1, updated.length()-pos_punct-1);
	    updated = before + after;
         } else {
            removing_punctuation = false;
         }
      }
      return updated;
   } else {
      return string(s);
   }
}
