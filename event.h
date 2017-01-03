#ifndef H_EVENT_H
#define H_EVENT_H

#include "avr-config.h"

#include <avr/io.h>

extern uint8_t event_do_chk_clr( uint8_t mask );

void event_wait( uint8_t mask );

static inline uint8_t event_chk_clr( uint8_t m )
{
   
   return (event_do_chk_clr( ~m ) & m);
}  

static inline void event_set_powerdown( uint8_t onoff )
{
   if (onoff) {
      MCUCR=(MCUCR&~_BV(SM1))|_BV(SM0)|_BV(SE);
   } else {
      MCUCR=(MCUCR&~(_BV(SM1)|_BV(SM0)))|_BV(SE);
   }
}

#ifndef EVENT_GPIO_REG

extern volatile uint8_t event_bits;

static inline void event_init(void)
{
   event_bits=0;
}

static inline void event_trigger( uint8_t m )
{
   register uint8_t val;
   asm( "in r0, __SREG__\n\t"
	"cli\n\t"
	"lds %[val], %[bits]\n\t"
	"ori %[val], %[mask]\n\t"
	"out __SREG__,r0\n\t"
	"sts %[bits], %[val]\n\t"
	: [val] "=a" (val), [bits] "=m" (event_bits)
	: [mask] "M" (m) );
}

static inline uint8_t event_chk( uint8_t m )
{
   return event_bits&m;
}

static inline void event_clr( uint8_t m )
{
   event_do_chk_clr( ~m );
}

#else

static inline void event_init(void)
{
   EVENT_GPIO_REG=0;
}

static inline void event_trigger( uint8_t m )
{
   EVENT_GPIO_REG |= m;
}

static inline uint8_t event_chk( uint8_t m )
{
   return EVENT_GPIO_REG & m;
}

static inline void event_clr( uint8_t m )
{
   EVENT_GPIO_REG &= ~m;
}
#endif

#define event_reload( d ) asm( "/* memory barrier */" : : "m" (d) );

#endif
