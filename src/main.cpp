#include <string>
#include <vector>

#include "jukebox_main.h"


int main(int argc, char* argv[])
{
   std::vector<std::string> args;
   for (int i = 1; i < argc; i++) {
      const char* arg = argv[i];
      args.push_back(std::string(arg));
   }

   JukeboxMain jb_main;
   return jb_main.run(args);
}

