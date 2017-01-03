#include "avr-config.h"
#include <avr/io.h>
#include "rotary.h"


#define	LOOP_COUNT 15
#define OFF_COUNT 5
#define ON_COUNT 11

static uint8_t count_a;
static uint8_t count_b;
static uint8_t countdown;

int8_t rotary_sample(void)
{
	int8_t ret=ROTARY_NONE;

	if (!(ROTARY_A_IPORT&_BV(ROTARY_A_PIN))) {
		count_a++;
	}
	if (!(ROTARY_B_IPORT&_BV(ROTARY_B_PIN))) {
		count_b++;
	}
	countdown--;
	if (countdown==0) {
		if (count_a>ON_COUNT) {
			/* so pin A was off, and is now on */
			if (count_b>ON_COUNT) {
				ret=ROTARY_CCW;
			} else if (count_b<OFF_COUNT) {
				ret=ROTARY_CW;
			}
			countdown=LOOP_COUNT|0x80;
		} else {
			countdown=LOOP_COUNT;
			ret=ROTARY_RESET; /* nothing was found */
		}
		count_a=0;
		count_b=0;
	} else if (countdown==0x80) {
		if (count_a<OFF_COUNT) {
			/* so pin A was on, and is now off */
			countdown=LOOP_COUNT;
		} else {
			countdown=LOOP_COUNT|0x80;
			ret=ROTARY_RESET; /* nothing was found */
		}
		count_a=0;
		count_b=0;
	}
	return ret;
}

void rotary_start(void)
{
	count_a=0;
	count_b=0;
	countdown=LOOP_COUNT;
}

void rotary_stop(void)
{
}

void rotary_init(void)
{
	/* make it inputs */
	ROTARY_A_DDR&=~_BV(ROTARY_A_PIN);
	ROTARY_B_DDR&=~_BV(ROTARY_B_PIN);
	ROTARY_A_PORT|=_BV(ROTARY_A_PIN);
	ROTARY_B_PORT|=_BV(ROTARY_B_PIN);
	rotary_start();
}



/*
 * Local Variables:
 * mode: c
 * c-file-style: "linux"
 * End:
 */
