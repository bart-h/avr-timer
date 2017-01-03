#include "avr-config.h"
#include "charlieplex.h"

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sfr_defs.h>
#include <avr/pgmspace.h>

#define mask0A CP_GET(0,CP_MASK0)
#define mask0B CP_GET(1,CP_MASK0)
#define mask0D CP_GET(3,CP_MASK0)
		
#define mask1A CP_GET(0,CP_MASK1)
#define mask1B CP_GET(1,CP_MASK1)
#define mask1D CP_GET(3,CP_MASK1)
   
#define mask2A CP_GET(0,CP_MASK2)
#define mask2B CP_GET(1,CP_MASK2)
#define mask2D CP_GET(3,CP_MASK2)
   
   
#define maskA (mask0A|mask1A|mask2A)
#define maskB (mask0B|mask1B|mask2B)
#define maskD (mask0D|mask1D|mask2D)



void cp_stop_display(void)
{
	PORTA&=~maskA;
	PORTB&=~maskB;
	PORTD&=~maskD;
	DDRA&=~maskA;
	DDRB&=~maskB;
	DDRD&=~maskD;
}

#define PAIR(a,b) (((a)<<4)|(b))

static const  struct GLOBAL_DATA {
	const uint8_t index[12];
	const uint8_t bitmaskA[4];
	const uint8_t bitmaskB[4];
	const uint8_t bitmaskD[4];
} data = {
	{
		PAIR(0,1),PAIR(2,1),
		PAIR(2,3),PAIR(0,3),
		PAIR(0,2),PAIR(2,0),
		PAIR(3,0),PAIR(1,3),
		PAIR(1,2),PAIR(3,2),
		PAIR(3,1),PAIR(1,0),
	},
	/* A */
	{
		CP_GET(0,CP_BIT00|CP_BIT10|CP_BIT20),
		CP_GET(0,CP_BIT01|CP_BIT11|CP_BIT21),
		CP_GET(0,CP_BIT02|CP_BIT12|CP_BIT22), 				
		CP_GET(0,CP_BIT03|CP_BIT13|CP_BIT23),
	},
	/* B */
	{
		CP_GET(1,CP_BIT00|CP_BIT10|CP_BIT20),
		CP_GET(1,CP_BIT01|CP_BIT11|CP_BIT21),
		CP_GET(1,CP_BIT02|CP_BIT12|CP_BIT22), 
		CP_GET(1,CP_BIT03|CP_BIT13|CP_BIT23),
	},
	/* D */
	{
		CP_GET(3,CP_BIT00|CP_BIT10|CP_BIT20),
		CP_GET(3,CP_BIT01|CP_BIT11|CP_BIT21),
		CP_GET(3,CP_BIT02|CP_BIT12|CP_BIT22),
		CP_GET(3,CP_BIT03|CP_BIT13|CP_BIT23),
	}
};



void cp_led_on( const uint8_t seq, const uint8_t ledmask)
/*
 * seq:     sequence number for led: 0..11
 * ledmask: 0x01 - enable first set of leds
 *          0x02 - enable second set of leds
 *          0x04 - enable third set of leds
 */
{
	const uint8_t ip=(data.index[seq]>>4)&0xf;
	const uint8_t im=(data.index[seq])&0xf;
	uint8_t tmaskA=0,tmaskB=0,tmaskD=0;
	uint8_t tmp;

	cp_stop_display();
	
	if (ledmask&1) {
		tmaskA|=mask0A;
		tmaskB|=mask0B;
		tmaskD|=mask0D;
	}
	if (ledmask&2) {
		tmaskA|=mask1A;
		tmaskB|=mask1B;
		tmaskD|=mask1D;
	}
	if (ledmask&4) {
		tmaskA|=mask2A;
		tmaskB|=mask2B;
		tmaskD|=mask2D;
	}

	tmp=PORTA & ~maskA;
	tmp|=data.bitmaskA[ip]&tmaskA;
	PORTA=tmp;
	tmp=DDRA & ~maskA;
	tmp|=data.bitmaskA[ip]|data.bitmaskA[im];
	DDRA=tmp;
	
	tmp=PORTB & ~maskB;
	tmp|=data.bitmaskB[ip]&tmaskB;
	PORTB=tmp;
	tmp=DDRB & ~maskB;
	tmp|=data.bitmaskB[ip]|data.bitmaskB[im];
	DDRB=tmp;
	
	tmp=PORTD & ~maskD;
	tmp|=data.bitmaskD[ip]&tmaskD;
	PORTD=tmp;	
	tmp=DDRD & ~maskD;
	tmp|=data.bitmaskD[ip]|data.bitmaskD[im];
	DDRD=tmp;
}




/*
 * Local Variables:
 * mode: c
 * c-file-style: "linux"
 * End:
 */

