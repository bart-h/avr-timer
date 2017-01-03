#ifndef H_BUZZER_H

#include <inttypes.h>
#include <avr/io.h>
#include <avr/pgmspace.h>

void buzzer_silence(void);
void buzzer_init(void);
void buzzer_click(void);

void buzzer_song_start(void);
int8_t buzzer_song_next(void);


/*
 * byte encoded song
 */
uint8_t song_get_byte( uint8_t index );

#endif

/*
 * Local Variables:
 * mode: c
 * c-file-style: "linux"
 * End:
 */
