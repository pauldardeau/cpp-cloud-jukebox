#ifndef ARGUMENT_PARSER_H
#define ARGUMENT_PARSER_H

#include <string>
#include <map>
#include <vector>

#include "data_types.h"

class PropertySet;


class ArgumentParser {
private:
   static const std::string TYPE_BOOL;
   static const std::string TYPE_INT;
   static const std::string TYPE_STRING;

   std::map<std::string, std::string> dict_all_reserved_words;
   std::map<std::string, std::string> dict_bool_options;
   std::map<std::string, std::string> dict_int_options;
   std::map<std::string, std::string> dict_string_options;
   std::map<std::string, std::string> dict_commands;
   std::vector<std::string> list_commands;

   ArgumentParser(const ArgumentParser&);
   ArgumentParser& operator=(const ArgumentParser&);

protected:
   void addOption(const std::string& o,
                  const std::string& option_type,
		  const std::string& help);

public:
   ArgumentParser();

   void addOptionalBoolFlag(const std::string& flag, const std::string& help);
   void addOptionalIntArgument(const std::string& arg, const std::string& help);
   void addOptionalStringArgument(const std::string& arg, const std::string& help);
   void addRequiredArgument(const std::string& arg, const std::string& help);
   PropertySet* parse_args(const std::vector<std::string>& args);
};

#endif

