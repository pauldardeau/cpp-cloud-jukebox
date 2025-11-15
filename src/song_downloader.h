#ifndef SONG_DOWNLOADER_H
#define SONG_DOWNLOADER_H

#include <vector>

#include "jukebox.h"
#include "song_metadata.h"
#include "Runnable.h"


class SongDownloader : public chaudiere::Runnable {
private:
   Jukebox& m_jukebox;
   std::vector<SongMetadata> m_list_songs;

   SongDownloader();
   SongDownloader(const SongDownloader&);
   SongDownloader& operator=(const SongDownloader&);

public:
   SongDownloader(Jukebox& jb, const std::vector<SongMetadata>& list_songs);
   virtual ~SongDownloader();

   virtual void run();
};

#endif
