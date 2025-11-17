#include "argument_parser.h"
#include "property_set.h"
#include "StrUtils.h"

using namespace std;
using namespace chaudiere;

const string ArgumentParser::TYPE_BOOL = "bool";
const string ArgumentParser::TYPE_INT = "int";
const string ArgumentParser::TYPE_STRING = "string";

//*****************************************************************************

ArgumentParser::ArgumentParser() {
}

//*****************************************************************************

ArgumentParser::~ArgumentParser() {
}

//*****************************************************************************

void ArgumentParser::addOption(const string& o,
                               const string& option_type,
                               const string& help) {
   m_dict_all_reserved_words[o] = option_type;

   if (option_type == TYPE_BOOL) {
      m_dict_bool_options[o] = help;
   } else if (option_type == TYPE_INT) {
      m_dict_int_options[o] = help;
   } else if (option_type == TYPE_STRING) {
      m_dict_string_options[o] = help;
   }
}

//*****************************************************************************

void ArgumentParser::addOptionalBoolFlag(const string& flag, const string& help) {
   addOption(flag, TYPE_BOOL, help);
}

//*****************************************************************************

void ArgumentParser::addOptionalIntArgument(const string& arg, const string& help) {
   addOption(arg, TYPE_INT, help);
}

//*****************************************************************************

void ArgumentParser::addOptionalStringArgument(const string& arg, const string& help) {
   addOption(arg, TYPE_STRING, help);
}

//*****************************************************************************

void ArgumentParser::addRequiredArgument(const string& arg, const string& help) {
   m_dict_commands[arg] = help;
   m_list_commands.push_back(arg);
}

//*****************************************************************************

PropertySet* ArgumentParser::parse_args(const vector<string>& args) {

   PropertySet* pset = new PropertySet;

   int num_args = args.size();
   bool working = true;
   int i = 0;
   unsigned int commands_found = 0;

   if (num_args == 0) {
      working = false;
   }

   while (working) {
      const string& arg = args[i];

      auto it = m_dict_all_reserved_words.find(arg);
      if (it != m_dict_all_reserved_words.end()) {
         const string& arg_type = it->second;
         string the_arg = arg.substr(2, arg.length()-2);
         if (arg_type == TYPE_BOOL) {
            pset->add(the_arg, new BoolPropertyValue(true));
         } else if (arg_type == TYPE_INT) {
            i++;
            if (i < num_args) {
               int int_value = StrUtils::parseInt(args[i]);
               pset->add(the_arg, new IntPropertyValue(int_value));
            } else {
               // missing int value
            }
         } else if (arg_type == TYPE_STRING) {
            i++;
            if (i < num_args) {
               const string& next_arg = args[i];
               pset->add(the_arg, new StrPropertyValue(next_arg));
            } else {
               // missing string value
            }
         } else {
            // unrecognized type
         }
      } else {
         // has "--" prefix?
         if (arg.rfind("--", 0) == 0) {
            // unrecognized option
         } else {
            if (commands_found < m_list_commands.size()) {
               const string& command_name = m_list_commands[commands_found];
               pset->add(command_name, new StrPropertyValue(arg));
               commands_found++;
            } else {
               // unrecognized command
            }
         }
      }

      i++;
      if (i >= num_args) {
         working = false;
      }
   }

   return pset;
}

//*****************************************************************************

