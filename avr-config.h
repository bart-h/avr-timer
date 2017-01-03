#ifndef H_AVR_CONFIG_H
#define H_AVR_CONFIG_H

#define F_CPU 1000000UL /* 1 MHz */

/*
 * PA0 - Leds
 * PA1 - Leds
 * PA2 - reset
 * PB0 - AIN1
 * PB1 - AIN0
 * PB2 - Rotary
 * PB3 - OC1A - Buzzer
 * PB4 - Rotary
 * PB5 - MOSI - Leds
 * PB6 - MISO - Leds
 * PB7 - UCSK - Leds
 * PD0 - Leds
 * PD1 - Leds
 * PD2 - Leds
 * PD3 - Leds
 * PD4 - Leds
 * PD5 - Leds
 * PD6 - Leds
 */


/*   Reset - PA2 - 1 --- 20 - VCC
 *   LEDC1 - PD0 - 2 --- 19 - PB7 - LEDC0 - UCSK
 *   LEDC2 - PD1 - 3 --- 18 - PB6 - LEDB0 - MISO
 *   LEDC3 - PA1 - 4 --- 17 - PB5 - LEDA0 - MOSI
 *   LEDB1 - PA0 - 5 --- 16 - PB4 - ROT0 
 *   LEDB2 - PD2 - 6 --- 15 - PB3 - Buzzer
 *   LEDB3 - PD3 - 7 --- 14 - PB2 - ROT1
 *   LEDA1 - PD4 - 8 --- 13 - PB1 - AIN1
 *   LEDA2 - PD5 - 9 --- 12 - PB0 - AIN0
 *           GND - 10 -- 11 - PD6 - LEDA3



/*
 * charlieplex configuration
 */
/*
 * bits:
 * PB7 PD0 PD1 PA1
 * PB6 PA0 PD2 PD3 
 * PB5 PD4 PD5 PD6
 * need: 3x4
 */
	
#define CP_VAL(PORT,BIT) (1LL<<((BIT)+(PORT)*8))

#define CP_BIT00 CP_VAL(1,5)
#define CP_BIT01 CP_VAL(3,4)
#define CP_BIT02 CP_VAL(3,5)
#define CP_BIT03 CP_VAL(3,6)
#define CP_MASK0 (CP_BIT00|CP_BIT01|CP_BIT02|CP_BIT03)	

#define CP_BIT10 CP_VAL(1,6)
#define CP_BIT11 CP_VAL(0,0)
#define CP_BIT12 CP_VAL(3,2)
#define CP_BIT13 CP_VAL(3,3)
#define CP_MASK1 (CP_BIT10|CP_BIT11|CP_BIT12|CP_BIT13)

#define CP_BIT20 CP_VAL(1,7)
#define CP_BIT21 CP_VAL(3,0)
#define CP_BIT22 CP_VAL(3,1)
#define CP_BIT23 CP_VAL(0,1)
#define CP_MASK2 (CP_BIT20|CP_BIT21|CP_BIT22|CP_BIT23)
	
#define CP_GET(PORT,VAL) (((VAL)>>((PORT)*8))&0xff)

#define ROTARY_A_IPORT PINB
#define ROTARY_A_PORT PORTB
#define ROTARY_A_DDR  DDRB
#define ROTARY_B_IPORT PINB
#define ROTARY_B_PORT PORTB
#define ROTARY_B_DDR  DDRB
#define ROTARY_A_PIN  PB2
#define ROTARY_B_PIN  PB4
#define ROTARY_PC_PIN_A PCINT2
#define ROTARY_PC_PIN_B PCINT4

#define BUZZER_PORT PORTB
#define BUZZER_DDR DDRB
#define BUZZER_PIN PB3

#define EVENT_GPIO_REG GPIOR0

#endif


/*
 * Local Variables:
 * mode: c
 * c-file-style: "linux"
 * End:
 */

