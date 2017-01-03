#include "avr-config.h"
#include "buzzer.h"

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sfr_defs.h>
#include <avr/pgmspace.h>


/*
 * Convert note frequency to scaling factor. Multiplied by 32 to 
 * make optimal use of 16 bits
 */

#define NS(freq) ((uint16_t)(F_CPU/((0.5+(freq))/4.0)))

static const uint16_t PROGMEM freq[12]=
{
	NS(523.25), /* C  (61185) */
	NS(554.37), /* C# */
	NS(587.33), /* D  */
	NS(622.25), /* D# */
	NS(659.25), /* E  */
	NS(698.46), /* F  */
	NS(739.99), /* F# */
	NS(783.99), /* G  */
	NS(830.61), /* G# */
	NS(880.00), /* A  */
	NS(932.33), /* A# */
	NS(987.77), /* B  (32421) */
};

uint16_t song_index;
uint8_t song_val;
uint8_t song_count;

/*
 * note format:
 *
 * bits 0-3: note 0-11
 * bits 6&7: octave: 0-3
 * bits 4&5: length 0-3
 */

static void buzzer_play(uint8_t note)
{
	uint16_t scaler;
	
	buzzer_silence();
	
	scaler=pgm_read_word(&freq[note&0x0f]);
	for(uint8_t cnt=note&0xc0; (cnt); cnt-=0x40) {
		scaler>>=1;
	}
	OCR1A=scaler;
	TCNT1=0;
	
	TCCR1A=_BV(COM1A0); /*  CTC - toggle on compare match */
	TCCR1B=_BV(WGM12)|_BV(CS10);
}

void buzzer_silence(void)
{
	TCCR1A=0; /* disconnect timer */
	TCCR1B=_BV(WGM12);
	BUZZER_PORT&=~_BV(BUZZER_PIN); /* zero */
}

void buzzer_click(void)
{
	TCCR1A=0; /* disconnect timer */
	TCCR1B=_BV(WGM12);
	BUZZER_PORT|=_BV(BUZZER_PIN); 
}

void buzzer_init(void)
{
	song_index=0;
	song_count=0;
	BUZZER_DDR|=_BV(BUZZER_PIN); /* enable output */
	buzzer_silence();
}


void buzzer_song_start(void)
{
	song_index=0;
	song_val=song_get_byte(0);
	buzzer_play(song_val);
	song_val&=0x3f; /* get rid of octaves.... */
}

int8_t buzzer_song_next(void)
{
	if (song_val&0xf0) {
		song_val-=0x10;
	} else {
		if (song_val==0x0f)
			return 0;
		/* next byte */
		++song_index;
		song_val=song_get_byte(song_index);
		switch(song_val&0x0f) {
		case 0x0f:
			/* done */
			buzzer_silence();
			return 0;
		case 0x0e:
			/* extension of previous. do nothing */
			break;
		case 0x0d:
			/* pause */
			buzzer_silence();
			break;
		default:
			buzzer_play(song_val);
			song_val&=0x3f;
		}
	}
	return 1;
}


/*
 * Local Variables:
 * mode: c
 * c-file-style: "linux"
 * End:
 */
