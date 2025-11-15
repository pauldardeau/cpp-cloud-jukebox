#ifndef JUKEBOX_MAIN_H
#define JUKEBOX_MAIN_H

#include <string>
#include <map>
#include <vector>

class StorageSystem;
class PropertySet;
class Jukebox;

class JukeboxMain {
private:
   std::string m_artist;
   std::string m_album;
   std::string m_song;
   std::string m_playlist;
   bool m_update_mode;
   bool m_debug_mode;

   JukeboxMain(const JukeboxMain&);
   JukeboxMain& operator=(const JukeboxMain&);

public:
   JukeboxMain();

   StorageSystem* connect_swift_system(const PropertySet& credentials,
                                       std::string prefix);

   StorageSystem* connect_s3_system(const PropertySet& credentials,
                                    std::string prefix,
                                    bool use_external);

   StorageSystem* connect_azure_system(const PropertySet& credentials,
                                       std::string prefix);

   StorageSystem* connect_memory_system(const PropertySet& credentials,
                                        std::string prefix);

   StorageSystem* connect_fs_system(const PropertySet& credentials,
                                    std::string prefix);

   StorageSystem* connect_storage_system(const std::string& system_name,
                                         const PropertySet& credentials,
                                         std::string prefix);

   bool init_storage_system(StorageSystem* storage_sys);

   void show_usage() const;

   int run(const std::vector<std::string>& console_args);

   int run_jukebox_command(Jukebox& jukebox, const std::string& command);
};

#endif

