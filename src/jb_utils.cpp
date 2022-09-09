#include "jb_utils.h"

using namespace std;

string JBUtils::unencode_value(const string& encoded_value) {
   return encoded_value.replace('-', ' ');
}

string JBUtils::encode_value(const string& value) {
   return value.replace(' ', '-');
}

