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
   //printf("SongDownloader.run\n");
   if (list_songs.size() > 0) {
      //printf("SongDownloader.run: call batch_download_start\n");
      jukebox.batch_download_start();

      auto it = list_songs.begin();
      const auto it_end = list_songs.end();

      for (; it != it_end; it++) {
         const SongMetadata& song = *it;
         if (jukebox.exit_requested) {
            //printf("SongDownloader.run: jukebox.exit_requested is true\n");
            break;
         } else {
            //printf("downloading song %s\n", song.song_name.c_str());
            jukebox.download_song(song);
         }
      }
      //printf("SongDownloader.run: call batch_download_complete\n");
      jukebox.batch_download_complete();
   } else {
      printf("SongDownloader.run: list_songs is empty\n");
   }
   //printf("************** SongDownloader thread completed\n");
}

