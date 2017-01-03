#include "avr-config.h"

#include "event.h"

#ifndef EVENT_GPIO_REG

volatile uint8_t event_bits;

uint8_t event_do_chk_clr( uint8_t mask )
{
   /* atomic */
   register uint8_t val;
   asm volatile ( "in r0, __SREG__\n\t"
		  "cli\n\t"
		  "lds %[val], %[bits]\n\t"
		  "and %[mask], %[val]\n\t"
		  "out __SREG__, r0\n\t"
		  "sts %[bits], %[mask]\n\t"
		  : [val] "=r" (val), [bits] "+m" (event_bits), [mask] "+r" (mask) );
   return val;
}

void event_wait( uint8_t mask )
{
   asm volatile( "cli\n\t"
	"lds r0, %[bits]\n\t"
	"and %[mask], r0\n\t"
	"brne 0f\n\t"
	"sei\n\t"
	"sleep\n\t"
	"0:\n\t"
	"sei"
	: [mask] "+r" (mask) : [bits] "m" (event_bits) );
}

#else
uint8_t event_do_chk_clr( uint8_t mask )   
{
   /* atomic */
   register uint8_t val;
   asm volatile ( "in r0, __SREG__\n\t"
		  "cli\n\t"
		  "in %[val], %[gpio]\n\t"
		  "and %[mask], %[val]\n\t"
		  "out __SREG__, r0\n\t"
		  "out %[gpio], %[mask]\n\t"
		  : [val] "=r" (val), [mask] "+r" (mask) : [gpio] "I" (_SFR_IO_ADDR(EVENT_GPIO_REG)) );
   return val;
}

void event_wait( uint8_t mask )
{
   asm volatile ( "cli\n\t"
	"in r0, %[gpio]\n\t"
	"and %[mask], r0\n\t"
	"brne 1f\n\t"
	"sei\n\t"
	"sleep\n\t"
	"1:\n\t"
	"sei"
	: [mask] "+r" (mask) : [gpio] "I" (_SFR_IO_ADDR(EVENT_GPIO_REG)) );
}

#endif

