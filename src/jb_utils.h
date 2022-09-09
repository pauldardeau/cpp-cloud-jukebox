#ifndef JB_UTILS_H
#define JB_UTILS_H

#include <string>


class JBUtils {
public:
   static std::string unencode_value(const std::string& encoded_value);
   static std::string encode_value(const std::string& value);
};

#endif

