#ifndef JUKEBOX_MAIN_H
#define JUKEBOX_MAIN_H

#include <string>
#include <map>
#include <vector>

class StorageSystem;
class PropertySet;


class JukeboxMain {
public:
   StorageSystem* connect_swift_system(const PropertySet& credentials,
                                       std::string prefix,
                                       bool in_debug_mode,
                                       bool in_update_mode);

   StorageSystem* connect_s3_system(const PropertySet& credentials,
                                    std::string prefix,
                                    bool in_debug_mode,
                                    bool in_update_mode);

   StorageSystem* connect_azure_system(const PropertySet& credentials,
                                       std::string prefix,
                                       bool in_debug_mode,
                                       bool in_update_mode);

   StorageSystem* connect_memory_system(const PropertySet& credentials,
                                        std::string prefix,
                                        bool in_debug_mode,
                                        bool in_update_mode);

   StorageSystem* connect_fs_system(const PropertySet& credentials,
                                    std::string prefix,
                                    bool in_debug_mode,
                                    bool in_update_mode);

   StorageSystem* connect_storage_system(const std::string& system_name,
                                         const PropertySet& credentials,
                                         std::string prefix,
                                         bool in_debug_mode,
                                         bool in_update_mode);

   void show_usage() const;

   void run(const std::vector<std::string>& console_args);
};

#endif

