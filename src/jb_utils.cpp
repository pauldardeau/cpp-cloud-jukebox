#include "jb_utils.h"
#include "StrUtils.h"

using namespace std;

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

