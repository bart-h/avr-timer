#include <avr/io.h>
#include <avr/pgmspace.h>

#include "buzzer.h"


static const uint8_t the_buzzer_song[] PROGMEM ={
#include "song-dat.inc"
  0x0f };

uint8_t song_get_byte( uint8_t index )
{
	return pgm_read_byte(&(the_buzzer_song[index]));
}
