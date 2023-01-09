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
