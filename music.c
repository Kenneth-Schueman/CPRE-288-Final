#include "music.h"
#include "open_interface.h"


#define IMERPIAL_MARCH 0

//loads song index of choice to the oi
void sound_load_marchTheme() {

      unsigned char ImperialMarchNumNotes = 19;
      unsigned char ImperialMarchNotes[19]     = {55, 55, 55, 51, 58, 55, 51, 58, 55, 0,  62, 62, 62, 63, 58, 54, 51, 58, 55};
      unsigned char ImperialMarchDurations[19] = {32, 32, 32, 20, 12, 32, 20, 12, 32, 32, 32, 32, 32, 20, 12, 32, 20, 12, 32};
      oi_loadSong(IMERPIAL_MARCH, ImperialMarchNumNotes, ImperialMarchNotes, ImperialMarchDurations);
}

//plays a song of choice numLoops times
void sound_play_marchTheme() {
    oi_play_song(IMERPIAL_MARCH);
}
