#include "song_downloader.h"

using namespace std;

//*****************************************************************************

SongDownloader::SongDownloader(Jukebox& jb,
                               const vector<SongMetadata>& the_song_list) :
   m_jukebox(jb),
   m_list_songs(the_song_list) {
}

//*****************************************************************************

SongDownloader::~SongDownloader() {
}

//*****************************************************************************

void SongDownloader::run() {
   if (!m_list_songs.empty()) {
      m_jukebox.batch_download_start();

      for (const auto& song : m_list_songs) {
         if (m_jukebox.is_exit_requested()) {
            break;
         } else {
            m_jukebox.download_song(song);
         }
      }
      m_jukebox.batch_download_complete();
   } else {
      printf("SongDownloader.run: list_songs is empty\n");
   }
}

//*****************************************************************************

