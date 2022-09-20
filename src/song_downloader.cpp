#include "song_downloader.h"

using namespace std;

SongDownloader::SongDownloader(Jukebox& jb,
                               const vector<SongMetadata>& the_song_list) :
   jukebox(jb),
   list_songs(the_song_list) {
}

SongDownloader::~SongDownloader() {
}

void SongDownloader::run() {
   if (list_songs.size() > 0) {
      jukebox.batch_download_start();

      auto it = list_songs.begin();
      const auto it_end = list_songs.end();

      for (; it != it_end; it++) {
         const SongMetadata& song = *it;
         if (jukebox.exit_requested) {
            break;
         } else {
            jukebox.download_song(song);
         }
      }
      jukebox.batch_download_complete();
   } else {
      printf("SongDownloader.run: list_songs is empty\n");
   }
}

