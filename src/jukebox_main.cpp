#include <stdio.h>

#include "jukebox_main.h"
#include "data_types.h"
#include "argument_parser.h"
#include "property_set.h"
#include "jukebox_options.h"
#include "jukebox.h"
#include "s3_storage_system.h"
#include "utils.h"
#include "OSUtils.h"
#include "StringTokenizer.h"
#include "StrUtils.h"
#include "fs_storage_system.h"

using namespace std;
using namespace chaudiere;


StorageSystem* JukeboxMain::connect_swift_system(const PropertySet& credentials,
                                                 string prefix,
                                                 bool in_debug_mode,
                                                 bool in_update_mode)
{
      //if (!swift.is_available()) {
      //   printf("error: swift is not supported on this system. please install swiftclient first.");
      //   Utils::sys_exit(1);
      //}

      string swift_auth_host = "";
      string swift_account = "";
      string swift_user = "";
      string swift_password = "";
      string update_swift_user = "";
      string update_swift_password = "";

      if (credentials.contains("swift_auth_host")) {
         swift_auth_host = credentials.get_string_value("swift_auth_host");
      }
      if (credentials.contains("swift_account")) {
         swift_account = credentials.get_string_value("swift_account");
      }
      if (credentials.contains("swift_user")) {
         swift_user = credentials.get_string_value("swift_user");
      }
      if (credentials.contains("swift_password")) {
         swift_password = credentials.get_string_value("swift_password");
      }
      if (credentials.contains("update_swift_user") &&
          credentials.contains("update_swift_password")) {

         update_swift_user = credentials.get_string_value("update_swift_user");
         update_swift_password = credentials.get_string_value("update_swift_password");
      }

      if (in_debug_mode) {
         printf("swift_auth_host=%s\n", swift_auth_host.c_str());
         printf("swift_account=%s\n", swift_account.c_str());
         printf("swift_user=%s\n", swift_user.c_str());
         printf("swift_password=%s\n", swift_password.c_str());
         if (update_swift_user.length() > 0 && update_swift_password.length() > 0) {
            printf("update_swift_user=%s\n", update_swift_user.c_str());
            printf("update_swift_password=%s\n", update_swift_password.c_str());
         }
      }

      if (swift_account.length() == 0 ||
          swift_user.length() == 0 ||
          swift_password.length() == 0) {

         printf("error: no swift credentials given. please specify swift_account, "
                "swift_user, and swift_password in credentials\n");
         Utils::sys_exit(1);
      }

      string user = "";
      string password = "";

      if (in_update_mode) {
         user = update_swift_user;
         password = update_swift_password;
      } else {
         user = swift_user;
         password = swift_password;
      }

      return NULL;
      //TODO: (3) hookup SwiftStorageSystem (connect_swift_system)
      //return new SwiftStorageSystem(swift_auth_host,
      //                              swift_account,
      //                              user,
      //                              password,
      //                              in_debug_mode);
}

StorageSystem* JukeboxMain::connect_s3_system(const PropertySet& credentials,
                                              string prefix,
                                              bool in_debug_mode,
                                              bool in_update_mode)
{
   //if (!s3.is_available()) {
   //   printf("error: s3 is not supported on this system. please install boto3 (s3 client) first.");
   //   Utils::sys_exit(1);
   //}

   string aws_access_key = "";
   string aws_secret_key = "";
   string update_aws_access_key = "";
   string update_aws_secret_key = "";

   if (credentials.contains("aws_access_key")) {
      aws_access_key = credentials.get_string_value("aws_access_key");
   }
   if (credentials.contains("aws_secret_key")) {
      aws_secret_key = credentials.get_string_value("aws_secret_key");
   }

   if (credentials.contains("update_aws_access_key") &&
       credentials.contains("update_aws_secret_key")) {

      update_aws_access_key = credentials.get_string_value("update_aws_access_key");
      update_aws_secret_key = credentials.get_string_value("update_aws_secret_key");
   }

   if (in_debug_mode) {
      printf("aws_access_key=%s\n", aws_access_key.c_str());
      printf("aws_secret_key=%s\n", aws_secret_key.c_str());
      if (update_aws_access_key.length() > 0 && update_aws_secret_key.length() > 0) {
         printf("update_aws_access_key=%s\n", update_aws_access_key.c_str());
         printf("update_aws_secret_key=%s\n", update_aws_secret_key.c_str());
      }
   }

   if (aws_access_key.length() == 0 || aws_secret_key.length() == 0) {
      printf("error: no s3 credentials given. please specify aws_access_key "
             "and aws_secret_key in credentials file\n");
      return NULL;
   } else {
      string access_key = "";
      string secret_key = "";

      if (in_update_mode) {
         access_key = update_aws_access_key;
         secret_key = update_aws_secret_key;
      } else {
         access_key = aws_access_key;
         secret_key = aws_secret_key;
      }

      printf("Creating S3StorageSystem\n");
      return new S3StorageSystem(access_key,
                                 secret_key,
                                 prefix,
                                 in_debug_mode);
   }
   return NULL;
}

StorageSystem* JukeboxMain::connect_azure_system(const PropertySet& credentials,
                                                 string prefix,
                                                 bool in_debug_mode,
                                                 bool in_update_mode)
{
      //if (!azure.is_available()) {
      //   printf("error: azure is not supported on this system. please install azure client first.");
      //   Utils::sys_exit(1);
      //}

      string azure_account_name = "";
      string azure_account_key = "";
      string update_azure_account_name = "";
      string update_azure_account_key = "";

      if (credentials.contains("azure_account_name")) {
         azure_account_name = credentials.get_string_value("azure_account_name");
      }
      if (credentials.contains("azure_account_key")) {
         azure_account_key = credentials.get_string_value("azure_account_key");
      }

      if (credentials.contains("update_azure_account_name") &&
          credentials.contains("update_azure_account_key")) {

         update_azure_account_name = credentials.get_string_value("update_azure_account_name");
         update_azure_account_key = credentials.get_string_value("update_azure_account_key");
      }

      if (in_debug_mode) {
         printf("azure_account_name=%s\n", azure_account_name.c_str());
         printf("azure_account_key=%s\n", azure_account_key.c_str());
         if (update_azure_account_name.length() > 0 && update_azure_account_key.length() > 0) {
            printf("update_azure_account_name=%s\n", update_azure_account_name.c_str());
            printf("update_azure_account_key=%s\n", update_azure_account_key.c_str());
         }
      }

      if (azure_account_name.length() == 0 || azure_account_key.length() == 0) {
         printf("error: no azure credentials given. please specify azure_account_name "
                "and azure_account_key in credentials file\n");
         Utils::sys_exit(1);
      } else {
         string account_name = "";
         string account_key = "";

         if (in_update_mode) {
            account_name = update_azure_account_name;
            account_key = update_azure_account_key;
         } else {
            account_name = azure_account_name;
            account_key = azure_account_key;
         }

         //TODO: (3) hookup AzureStorageSystem (connect_azure_system)
         //return new AzureStorageSystem(account_name,
         //                              account_key,
         //                              prefix,
         //                              in_debug_mode);
      }

      return NULL;
}

StorageSystem* JukeboxMain::connect_fs_system(const PropertySet& credentials,
                                              string prefix,
                                              bool in_debug_mode,
                                              bool in_update_mode) {
   if (credentials.contains("root_dir")) {
      const string& root_dir = credentials.get_string_value("root_dir");
      if (in_debug_mode) {
         printf("root_dir = '%s'\n", root_dir.c_str());
      }
      return new FSStorageSystem(root_dir, in_debug_mode);
   } else {
      printf("error: 'root_dir' must be specified in fs_creds.txt\n");
      return NULL;
   }
}

StorageSystem* JukeboxMain::connect_storage_system(const string& system_name,
                                                   const PropertySet& credentials,
                                                   string prefix,
                                                   bool in_debug_mode,
                                                   bool in_update_mode) {
   if (system_name == "swift") {
      return connect_swift_system(credentials, prefix, in_debug_mode, in_update_mode);
   } else if (system_name == "s3") {
      return connect_s3_system(credentials, prefix, in_debug_mode, in_update_mode);
   } else if (system_name == "azure") {
      return connect_azure_system(credentials, prefix, in_debug_mode, in_update_mode);
   } else if (system_name == "fs") {
      return connect_fs_system(credentials, prefix, in_debug_mode, in_update_mode);
   } else {
      printf("error: unrecognized storage system %s\n", system_name.c_str());
      return NULL;
   }
}

void JukeboxMain::init_storage_system(StorageSystem* storage_sys) {
   if (Jukebox::initialize_storage_system(*storage_sys)) {
      printf("storage system successfully initialized\n");
   } else {
      printf("error: unable to initialize storage system\n");
      Utils::sys_exit(1);
   }
}

void JukeboxMain::show_usage() const {
   printf("Supported Commands:\n");
   printf("\tdelete-artist      - delete specified artist\n");
   printf("\tdelete-album       - delete specified album\n");
   printf("\tdelete-playlist    - delete specified playlist\n");
   printf("\tdelete-song        - delete specified song\n");
   printf("\thelp               - show this help message\n");
   printf("\timport-songs       - import all new songs from song-import subdirectory\n");
   printf("\timport-playlists   - import all new playlists from playlist-import subdirectory\n");
   printf("\timport-album-art   - import all album art from album-art-import subdirectory\n");
   printf("\tinit-storage       - initialize storage system\n");
   printf("\tlist-songs         - show listing of all available songs\n");
   printf("\tlist-artists       - show listing of all available artists\n");
   printf("\tlist-containers    - show listing of all available storage containers\n");
   printf("\tlist-albums        - show listing of all available albums\n");
   printf("\tlist-genres        - show listing of all available genres\n");
   printf("\tlist-playlists     - show listing of all available playlists\n");
   printf("\tshow-playlist      - show songs in specified playlist\n");
   printf("\tplay               - start playing songs\n");
   printf("\tshuffle-play       - play songs randomly\n");
   printf("\tplay-playlist      - play specified playlist\n");
   printf("\tretrieve-catalog   - retrieve copy of music catalog\n");
   printf("\tupload-metadata-db - upload SQLite metadata\n");
   printf("\tusage              - show this help message\n");
   printf("\n");
}

void JukeboxMain::run(const vector<string>& console_args) {
   int exit_code = 0;
   bool debug_mode = false;
   string storage_type = "swift";
   string artist = "";
   bool shuffle = false;
   string playlist;
   string song = "";
   string album = "";

   ArgumentParser opt_parser;
   opt_parser.addOptionalBoolFlag("--debug", "run in debug mode");
   opt_parser.addOptionalIntArgument("--file-cache-count", "number of songs to buffer in cache");
   opt_parser.addOptionalBoolFlag("--integrity-checks", "check file integrity after download");
   opt_parser.addOptionalBoolFlag("--compress", "use gzip compression");
   opt_parser.addOptionalBoolFlag("--encrypt", "encrypt file contents");
   opt_parser.addOptionalStringArgument("--key", "encryption key");
   opt_parser.addOptionalStringArgument("--keyfile", "path to file containing encryption key");
   opt_parser.addOptionalStringArgument("--storage", "storage system type (s3, swift, azure)");
   opt_parser.addOptionalStringArgument("--artist", "limit operations to specified artist");
   opt_parser.addOptionalStringArgument("--playlist", "limit operations to specified playlist");
   opt_parser.addOptionalStringArgument("--song", "limit operations to specified song");
   opt_parser.addOptionalStringArgument("--album", "limit operations to specified album");
   opt_parser.addRequiredArgument("command", "command for jukebox");

   PropertySet* args = opt_parser.parse_args(console_args);
   if (args == NULL) {
      printf("error: unable to obtain command-line arguments\n");
      Utils::sys_exit(1);
   }

   JukeboxOptions options;

   if (args->contains("debug")) {
      debug_mode = true;
      options.debug_mode = true;
   }

   if (args->contains("file_cache_count")) {
      int file_cache_count = args->get_int_value("file_cache_count");
      if (debug_mode) {
         printf("setting file cache count=%d\n", file_cache_count);
      }
      options.file_cache_count = file_cache_count;
   }

   if (args->contains("integrity_checks")) {
      if (debug_mode) {
         printf("setting integrity checks on\n");
      }
      options.check_data_integrity = true;
   }

   if (args->contains("compress")) {
      if (debug_mode) {
         printf("setting compression on\n");
      }
      options.use_compression = true;
   }

   if (args->contains("encrypt")) {
      if (debug_mode) {
         printf("setting encryption on\n");
      }
      options.use_encryption = true;
   }

   if (args->contains("key")) {
      string key = args->get_string_value("key");
      if (debug_mode) {
         printf("setting encryption key=%s\n", key.c_str());
      }
      options.encryption_key = key;
   }

   if (args->contains("keyfile")) {
      string keyfile = args->get_string_value("keyfile");
      if (debug_mode) {
         printf("reading encryption key file=%s\n", keyfile.c_str());
      }

      string encryption_key;
      if (Utils::file_read_all_text(keyfile, encryption_key) &&
          encryption_key.length() > 0) {

         options.encryption_key = StrUtils::strip(encryption_key);
      } else {
         printf("error: unable to read key file %s\n", keyfile.c_str());
         Utils::sys_exit(1);
      }

      if (options.encryption_key.length() == 0) {
         printf("error: no key found in file %s\n", keyfile.c_str());
         Utils::sys_exit(1);
      }
   }

   if (args->contains("storage")) {
      const string& storage = args->get_string_value("storage");
      StringSet supported_systems;
      supported_systems.add("swift");
      supported_systems.add("s3");
      supported_systems.add("azure");
      supported_systems.add("fs");
      if (!supported_systems.contains(storage)) {
         printf("error: invalid storage type %s\n", storage.c_str());
         printf("supported systems are: %s\n", supported_systems.to_string().c_str());
         Utils::sys_exit(1);
      } else {
         if (debug_mode) {
            printf("setting storage system to %s\n", storage.c_str());
         }
         storage_type = storage;
      }
   }

   if (args->contains("artist")) {
      artist = args->get_string_value("artist");
   }

   if (args->contains("playlist")) {
      playlist = args->get_string_value("playlist");
   }

   if (args->contains("song")) {
      song = args->get_string_value("song");
   }

   if (args->contains("album")) {
      album = args->get_string_value("album");
   }

   if (args->contains("command")) {
      if (debug_mode) {
         printf("using storage system type %s\n", storage_type.c_str());
      }

      string container_prefix = "com.swampbits.jukebox.";
      string creds_file = storage_type + "_creds.txt";
      PropertySet creds;
      string cwd = OSUtils::getCurrentDirectory();
      string creds_file_path = chaudiere::OSUtils::pathJoin(cwd, creds_file);

      if (Utils::path_exists(creds_file_path)) {
         if (debug_mode) {
            printf("reading creds file %s\n", creds_file_path.c_str());
         }

         string file_contents;

         bool file_read = Utils::file_read_all_text(creds_file_path,
                                                    file_contents);
         if (file_read && file_contents.length() > 0) {
            chaudiere::StringTokenizer st(file_contents, "\n");

	    while (st.hasMoreTokens()) {
               const string& file_line = st.nextToken();
	       vector<string> line_tokens = StrUtils::split(file_line, "=");
	       if (line_tokens.size() == 2) {
                  string key = chaudiere::StrUtils::strip(line_tokens[0]);
		  string value = chaudiere::StrUtils::strip(line_tokens[1]);
		  if (key.length() > 0 && value.length() > 0) {
                     creds.add(key, new StrPropertyValue(value));
                  }
               }
            }
         } else {
            if (debug_mode) {
               printf("error: unable to read file %s\n", creds_file_path.c_str());
            }
         }
      } else {
         printf("no creds file (%s)\n", creds_file_path.c_str());
      }

      options.encryption_iv = "sw4mpb1ts.juk3b0x";

      const string& command = args->get_string_value("command");

      StringSet help_cmds;
      help_cmds.add("help");
      help_cmds.add("usage");

      StringSet non_help_cmds;
      non_help_cmds.add("import-songs");
      non_help_cmds.add("play");
      non_help_cmds.add("shuffle-play");
      non_help_cmds.add("list-songs");
      non_help_cmds.add("list-artists");
      non_help_cmds.add("list-containers");
      non_help_cmds.add("list-genres");
      non_help_cmds.add("list-albums");
      non_help_cmds.add("retrieve-catalog");
      non_help_cmds.add("import-playlists");
      non_help_cmds.add("list-playlists");
      non_help_cmds.add("show-playlist");
      non_help_cmds.add("play-playlist");
      non_help_cmds.add("delete-song");
      non_help_cmds.add("delete-album");
      non_help_cmds.add("delete-playlist");
      non_help_cmds.add("delete-artist");
      non_help_cmds.add("upload-metadata-db");
      non_help_cmds.add("import-album-art");

      StringSet update_cmds;
      update_cmds.add("import-songs");
      update_cmds.add("import-playlists");
      update_cmds.add("delete-song");
      update_cmds.add("delete-album");
      update_cmds.add("delete-playlist");
      update_cmds.add("delete-artist");
      update_cmds.add("upload-metadata-db");
      update_cmds.add("import-album-art");
      update_cmds.add("init-storage");

      StringSet all_cmds;
      all_cmds.append(help_cmds);
      all_cmds.append(non_help_cmds);
      all_cmds.append(update_cmds);

      if (!all_cmds.contains(command)) {
         printf("Unrecognized command %s\n", command.c_str());
         printf("\n");
         show_usage();
      } else {
         if (help_cmds.contains(command)) {
            show_usage();
         } else {
            if (!options.validate_options()) {
               Utils::sys_exit(1);
            }

            StorageSystem* storage_system = NULL;

            try
            {
               if (command == "upload-metadata-db") {
                  options.suppress_metadata_download = true;
               } else {
                  options.suppress_metadata_download = false;
               }

               bool in_update_mode = false;

               if (update_cmds.contains(command)) {
                  in_update_mode = true;
               }

               storage_system = connect_storage_system(storage_type,
                                                       creds,
                                                       container_prefix,
                                                       debug_mode,
                                                       in_update_mode);
               if (storage_system == NULL) {
                  printf("error: unable to connect to storage system\n");
                  Utils::sys_exit(1);
               }

               printf("entering storage system\n");
               storage_system->enter();

	       if (command == "init-storage") {
                  init_storage_system(storage_system);
                  Utils::sys_exit(0);
	       }

               printf("creating jukebox\n");
               Jukebox jukebox(options, *storage_system);
               printf("entering jukebox\n");
               jukebox.enter();

               try {
                  if (command == "import-songs") {
                     jukebox.import_songs();
                  } else if (command == "import-playlists") {
                     jukebox.import_playlists();
                  } else if (command == "play") {
                     shuffle = false;
                     jukebox.play_songs(shuffle, artist, album);
                  } else if (command == "shuffle-play") {
                     shuffle = true;
                     jukebox.play_songs(shuffle, artist, album);
                  } else if (command == "list-songs") {
                     jukebox.show_listings();
                  } else if (command == "list-artists") {
                     jukebox.show_artists();
                  } else if (command == "list-containers") {
                     jukebox.show_list_containers();
                  } else if (command == "list-genres") {
                     jukebox.show_genres();
                  } else if (command == "list-albums") {
                     jukebox.show_albums();
                  } else if (command == "list-playlists") {
                     jukebox.show_playlists();
                  } else if (command == "show-playlist") {
                     if (playlist.length() > 0) {
                        jukebox.show_playlist(playlist);
                     } else {
                        printf("error: playlist must be specified using --playlist option\n");
                        exit_code = 1;
                     }
                  } else if (command == "play-playlist") {
                     if (playlist.length() > 0) {
                        jukebox.play_playlist(playlist);
                     } else {
                        printf("error: playlist must be specified using --playlist option\n");
                        exit_code = 1;
                     }
                  } else if (command == "retrieve-catalog") {
                     printf("retrieve-catalog not yet implemented\n");
                  } else if (command == "delete-song") {
                     if (song.length() > 0) {
                        if (jukebox.delete_song(song)) {
                           printf("song deleted\n");
                        } else {
                           printf("error: unable to delete song\n");
                           exit_code = 1;
                        }
                     } else {
                        printf("error: song must be specified using --song option\n");
                        exit_code = 1;
                     }
                  } else if (command == "delete-artist") {
                     if (artist.length() > 0) {
                        if (jukebox.delete_artist(artist)) {
                           printf("artist deleted\n");
                        } else {
                           printf("error: unable to delete artist\n");
                           exit_code = 1;
                        }
                     } else {
                        printf("error: artist must be specified using --artist option\n");
                        exit_code = 1;
                     }
                  } else if (command == "delete-album") {
                     if (album.length() > 0) {
                        if (jukebox.delete_album(album)) {
                           printf("album deleted\n");
                        } else {
                           printf("error: unable to delete album\n");
                           exit_code = 1;
                        }
                     } else {
                        printf("error: album must be specified using --album option\n");
                        exit_code = 1;
                     }
                  } else if (command == "delete-playlist") {
                     if (playlist.length() > 0) {
                        if (jukebox.delete_playlist(playlist)) {
                           printf("playlist deleted\n");
                        } else {
                           printf("error: unable to delete playlist\n");
                           exit_code = 1;
                        }
                     } else {
                        printf("error: playlist must be specified using --playlist option\n");
                        exit_code = 1;
                     }
                  } else if (command == "upload-metadata-db") {
                     if (jukebox.upload_metadata_db()) {
                        printf("metadata db uploaded\n");
                     } else {
                        printf("error: unable to upload metadata db\n");
                        exit_code = 1;
                     }
                  } else if (command == "import-album-art") {
                     jukebox.import_album_art();
                  }
               }
               catch (exception& e)
               {
                  printf("exception caught: %s\n", e.what());
               }
               catch (...)
               {
                  printf("unknown exception caught\n");
               }

               jukebox.exit();
               if (storage_system != NULL) {
                  storage_system->exit();
               }
            }
            catch (exception& e)
            {
               printf("exception caught: %s\n", e.what());
               exit_code = 1;
            }
         }
      }
   } else {
      printf("Error: no command given\n");
      show_usage();
   }
   Utils::sys_exit(exit_code);
}

