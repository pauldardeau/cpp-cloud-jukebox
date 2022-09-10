#include "argument_parser.h"
#include "property_set.h"

using namespace std;

const string ArgumentParser::TYPE_BOOL = "bool";
const string ArgumentParser::TYPE_INT = "int";
const string ArgumentParser::TYPE_STRING = "string";

ArgumentParser::ArgumentParser() {
}

void ArgumentParser::addOption(const string& o,
                               const string& option_type,
                               const string& help) {
   dict_all_reserved_words[o] = option_type;

   if (option_type == TYPE_BOOL) {
      dict_bool_options[o] = help;
   } else if (option_type == TYPE_INT) {
      dict_int_options[o] = help;
   } else if (option_type == TYPE_STRING) {
      dict_string_options[o] = help;
   }
}

void ArgumentParser::addOptionalBoolFlag(const string& flag, const string& help) {
   addOption(flag, TYPE_BOOL, help);
}

void ArgumentParser::addOptionalIntArgument(const string& arg, const string& help) {
   addOption(arg, TYPE_INT, help);
}

void ArgumentParser::addOptionalStringArgument(const string& arg, const string& help) {
   addOption(arg, TYPE_STRING, help);
}

void ArgumentParser::addRequiredArgument(const string& arg, const string& help) {
   dict_commands[arg] = help;
   list_commands.push_back(arg);
}

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

      auto it = dict_all_reserved_words.find(arg);
      if (it != dict_all_reserved_words.end()) {
         const string& arg_type = it->second;
         string the_arg = arg.substr(2, arg.length()-2);
         if (arg_type == TYPE_BOOL) {
            printf("adding key=%s value=true\n", the_arg.c_str());
            pset->add(the_arg, new BoolPropertyValue(true));
         } else if (arg_type == TYPE_INT) {
            i++;
            if (i < num_args) { 
               const string& next_arg = args[i];
               int int_value = atoi(next_arg.c_str());
               printf("adding key=%s value=%d\n", the_arg.c_str(), int_value);
               pset->add(the_arg, new IntPropertyValue(int_value));
            } else {
               // missing int value
            }
         } else if (arg_type == TYPE_STRING) {
            i++;
            if (i < num_args) {
               const string& next_arg = args[i];
               printf("adding key=%s value=%s\n", the_arg.c_str(), next_arg.c_str());
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
            if (commands_found < list_commands.size()) {
               const string& command_name = list_commands[commands_found];
               printf("adding key=%s value=%s\n", command_name.c_str(), arg.c_str());
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
