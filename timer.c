#include "avr-config.h"
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <inttypes.h>

#include "charlieplex.h"
#include "event.h"
#include "buzzer.h"
#include "rotary.h"

/*
 * General:
 * If the AVR is awake, a timer0 interrupt is triggered at 1.2 kHz. This is the master interrupt.
 * A variable "state" determines the state that the system is in (running,adjusting,etc).
 * The master interrupt samples the rotary encoder and after N samples, it decides if has turned.
 * This is done for debouncing.
 * In most states, the display is also updated.
 * The ISR uses a divisor counter to divide the 1.2 kHz into 0.2 second intervals (15 for running).
 * After such an interval, a "TICK" event is generated.
 *
 * These events are received by the mainloop, and used for timing/timeout.
 */

/*
 * Frequencies:
 * 
 * Refresh leds at 220 Hz (so every led has 4 pulses 55 times per second
 * 
 * Total frequency: ~2640 Hz
 * 
 * STATE_RUNNING: 15 seconds per tick: 15*25 loops of the beat
 * STATE_ALARM: 0.2 seconds per tick: 0.2*25 loops of the beat
 * STATE_ADJUSTCLOCK: 0.2 seconds per tick: 0.2*25 loops
 * STATE_ADJUSTALARM: 0.2 seconds per tick
 * 
 */

/* three separate charlieplex networks */
#define NUM_SETS 3
/* 12 leds per set (network) */
#define NUM_LEDS_PER_SET 12
/* 4 intensity-steps per led (duty-cycled) */
#define LED_INTENSITY_STEPS 4
/* base refresh frequency */
#define FREQ_LED_REFRESH 55
/* base frequency: 1200 Hz */
#define FREQ_BASE_CLOCK (FREQ_LED_REFRESH*LED_INTENSITY_STEPS*NUM_LEDS_PER_SET)

/* maximum range of clock */
#define MAX_CLOCK (NUM_SETS*NUM_LEDS_PER_SET*LED_INTENSITY_STEPS)

/* maximum range of refresh */
#define MAX_REFRESH (NUM_LEDS_PER_SET*LED_INTENSITY_STEPS)

/* TICK delay in ms */
#define TICK_DELAY (100)

#define MAX_DIVISOR ((FREQ_BASE_CLOCK/MAX_REFRESH*TICK_DELAY)/1000)

#if 1
/* maximum ticks while running: 25 seconds */
#define MAX_TICK_RUNNING (25000/TICK_DELAY)
#else
#define MAX_TICK_RUNNING (500/TICK_DELAY)
#endif

/* maximum ticks while alarm is sounding: 5 */
#define MAX_TICK_ALARM (15000/TICK_DELAY)

/* maximum ticks while adjusting the clock: 5 seconds */
#define MAX_TICK_ADJUSTCLOCK (5000/TICK_DELAY)

/* maximum ticks while adjusting the alarm: 5 seconds */
#define MAX_TICK_ADJUSTALARM (5000/TICK_DELAY)

enum {
	STATE_SLEEP,
	STATE_RUNNING,
	STATE_ALARM,
	STATE_ADJUSTCLOCK,
	STATE_ADJUSTALARM,
	STATE_DEBOUNCE,
	STATE_NONE,
};

volatile uint8_t state=STATE_SLEEP;
uint8_t clock_counter; /* clock_counter, adjusted by wheel */

uint8_t refresh_counter; /* refresh counter */
uint16_t divisor_counter; /* divisor counter */

uint8_t tick_counter; /* master timer ticks for a given state */
uint8_t silent=0; /* silent alarm? */

enum {
	EVENT_NONE=0x0,
	EVENT_TICK=0x01,
	EVENT_TURN_UP=0x02,
	EVENT_TURN_DOWN=0x04,
	EVENT_WAKEUP=0x08,
	EVENT_SLEEP=0x10,
	EVENT_ALL=0xff,
};

static void do_handle_display( uint8_t refresh, uint8_t count, uint8_t ledmask )
/*
 * refresh: runs from 0...MAX_REFRESH-1
 *          12*4
 * count: current counter setting. Runs from 0 to 36*4
 */
{
	uint8_t led=refresh>>2;

	if (refresh<count)
		ledmask^=0x01;
	if ((uint8_t)(refresh+48)<count)
		ledmask^=0x02;
	if ((uint8_t)(refresh+96)<count)
		ledmask^=0x04;
#if 1
	if (refresh&0x03) {
		uint8_t cled=(uint8_t)(count-1)>>2;

		if (led!=cled)
			ledmask&=~0x01;
		if ((uint8_t)(led+12)!=cled)
			ledmask&=~0x02;
		if ((uint8_t)(led+24)!=cled)
			ledmask&=~0x04;
	}
#endif
	cp_led_on(led,ledmask);
}

void handle_display_running(uint8_t refresh, uint8_t count )
{
	do_handle_display(refresh, count, 0 );
}

void handle_display_set_alarm(uint8_t refresh, uint8_t count )
{
	do_handle_display(refresh, MAX_CLOCK-count, 0x07 );
}

void handle_display_set_clock(uint8_t refresh, uint8_t count )
{
	do_handle_display(refresh, count, 0 );
}
	
void handle_display_alarm(uint8_t refresh, uint8_t ticks )
{
	uint8_t led=(ticks>>2)&0xf;

	if (led<12) {
		if (((uint8_t)refresh>>2)==led) {
			cp_led_on(led,0x07);
		} else {
			cp_led_on(led,0x00);
		}
	} else {
		do_handle_display(refresh, led&1 ? 0xff : 0x00, 0 );
	}
}

static void click_for_adjust(void)
{
	if (!silent) {
		buzzer_click();
	}
}

static void click_silence(void)
{
	buzzer_silence();
}


void timer_start(void)
{
	divisor_counter=MAX_DIVISOR-1;
	refresh_counter=MAX_REFRESH-1;
	/* reset counter */
	TCNT0=0;
	/* enable interrupt */
	TIMSK|=_BV(OCIE0A);
	//TCCR0B=_BV(CS02)|_BV(CS00); /* run with /1024 prescaling */
	TCCR0B=_BV(CS01); /* run with /8 prescaling */

}

void timer_stop(void)
{
	TIMSK&=~_BV(OCIE0A); /* disable interrupt */
	TCCR0B=0;            /* stop clock */
}

void timer_init(void)
{
	OCR0A=F_CPU/FREQ_BASE_CLOCK/8;
	TCCR0A=_BV(WGM01); /* CTC and no external pins */
	TIMSK=0;
	timer_stop();
}

static void wakeup_from_sleep(void)
{
	rotary_start();
	timer_start();
	buzzer_silence();
	event_set_powerdown(0); /* make sure we wake on timer interrupts */
}


void switch_state_running(void)
{
	tick_counter=MAX_TICK_RUNNING;
	state=STATE_RUNNING;
}

void switch_state_sleep(void)
{
	uint8_t msk;
	/* really go to sleep */
	timer_stop(); /* timer will stop: only knob may wake us */
	buzzer_silence();
	cp_stop_display();
	rotary_stop();
	event_set_powerdown(1);
	/* toggle which pin wakes us up. So if one of them is iffy, we 
	 * do not keep waking up
	 */
	msk=PCMSK;
	if (msk & _BV(ROTARY_PC_PIN_A)) {
		msk=(msk&~_BV(ROTARY_PC_PIN_A))|_BV(ROTARY_PC_PIN_B);
	} else {
		msk=(msk&~_BV(ROTARY_PC_PIN_B))|_BV(ROTARY_PC_PIN_A);
	}
	PCMSK=msk;
	GIMSK|=_BV(PCIE);
	state=STATE_SLEEP;
}

void switch_state_alarm(void)
{
	tick_counter=MAX_TICK_ALARM;
	state=STATE_ALARM;
	if (!silent)
		buzzer_song_start();
}

void switch_state_adjustclock(void)
{
	tick_counter=MAX_TICK_ADJUSTCLOCK;
	state=STATE_ADJUSTCLOCK;
}

void switch_state_adjustalarm(void)
{
	tick_counter=MAX_TICK_ADJUSTALARM;
	state=STATE_ADJUSTALARM;
}

void switch_state_debounce(void)
{
	tick_counter=MAX_TICK_RUNNING;
	state=STATE_DEBOUNCE; /* go to debounce mode */
}

ISR(TIMER0_COMPA_vect)
{
	/* display */
	switch(state) {
	case STATE_RUNNING:
		handle_display_running( refresh_counter, clock_counter );
		break;
	case STATE_ADJUSTCLOCK:
		handle_display_set_clock( refresh_counter, clock_counter );
		break;
	case STATE_ADJUSTALARM:
		handle_display_set_alarm( refresh_counter, clock_counter );
		break;
	case STATE_ALARM:
		handle_display_alarm( refresh_counter, tick_counter );
		break;
	}
	/* count */
	if (refresh_counter!=0) {
		--refresh_counter;
	} else {
		refresh_counter=MAX_REFRESH-1; /* restart loop */
		if (divisor_counter!=0) {
			--divisor_counter;
		} else {
			event_trigger(EVENT_TICK);
			divisor_counter=MAX_DIVISOR-1;
		} 
	}
	/* check rotary encoder */
	switch(rotary_sample()) {
	case ROTARY_RESET:
		if (state==STATE_DEBOUNCE) {
			/* we only woke up to check the rotary encoder,
			 * and it is not working-->back to sleep
			 */
			event_trigger(EVENT_SLEEP);
		}
		break;
	case ROTARY_CW:
		event_trigger(EVENT_TURN_UP);
		break;
	case ROTARY_CCW:
		event_trigger(EVENT_TURN_DOWN);
		break;
	}
}

ISR(PCINT_vect)
{
	/* this only ever gets called when we are asleep and
	 * the rotary encoder signals.
	 */
	GIMSK&=~_BV(PCIE); /* disable PC interrupt */
	event_trigger(EVENT_WAKEUP);
}

void mainloop(void)
{
	uint8_t event;
	event_wait(EVENT_ALL);
	event=event_chk_clr(EVENT_TURN_UP|EVENT_TURN_DOWN|EVENT_TICK|EVENT_WAKEUP|EVENT_SLEEP);
	if (event&EVENT_TURN_UP) {
		switch(state) {
		case STATE_ADJUSTCLOCK:
			tick_counter=MAX_TICK_ADJUSTCLOCK;
			clock_counter+=4;
			if (clock_counter>MAX_CLOCK)
				clock_counter=MAX_CLOCK;
			click_for_adjust();
			break;
		case STATE_ADJUSTALARM:
			clock_counter=0;
			switch_state_adjustclock();
			click_for_adjust();
			break;
		case STATE_RUNNING:
			switch_state_adjustclock();
			click_for_adjust();
			break;
		case STATE_DEBOUNCE:
			clock_counter=0;
			switch_state_adjustclock();
			click_for_adjust();
			break;
		case STATE_ALARM:
			switch_state_sleep();
			break;
		}
	}
	if (event&EVENT_TURN_DOWN) {
		switch(state) {
		case STATE_ADJUSTCLOCK:
			tick_counter=MAX_TICK_ADJUSTCLOCK;
			if (clock_counter>=4)
				clock_counter-=4;
			else if (clock_counter==0)
				switch_state_adjustalarm();
			else
				clock_counter=0;
			click_for_adjust();
			break;
		case STATE_ADJUSTALARM:
			tick_counter=MAX_TICK_ADJUSTALARM;
			clock_counter+=4;
			if (clock_counter>16) {
				silent=!silent;
				clock_counter=0;
				switch_state_adjustclock();
			}
			click_for_adjust();
			break;
		case STATE_RUNNING:
			switch_state_adjustclock();
			click_for_adjust();
			break;
		case STATE_DEBOUNCE:
			clock_counter=0;
			switch_state_adjustclock();
		        click_for_adjust();
			break;
		case STATE_ALARM:
			switch_state_sleep();
			break;
		}
	}
	if (event&EVENT_TICK) {
		if (--tick_counter==0) {
			switch(state) {
			case STATE_RUNNING:
				tick_counter=MAX_TICK_RUNNING;
				if (clock_counter!=0) {
					--clock_counter;
				} 
				if (clock_counter==0) {
					switch_state_alarm();
				}
				break;
			case STATE_ALARM:
				tick_counter=MAX_TICK_ALARM;
				if (silent)
					switch_state_sleep();
				break;
			case STATE_ADJUSTCLOCK:
				if (clock_counter==0)
					switch_state_sleep();
				else
					switch_state_running();
				break;
			case STATE_ADJUSTALARM:
				switch_state_sleep();
				break;
			case STATE_DEBOUNCE:
				/* more or less ignore this */
				tick_counter=MAX_TICK_RUNNING;
				break;
			}
		} else {
			switch(state) {
			case STATE_ALARM:
				if (!silent)
					if (buzzer_song_next()==0)
						switch_state_sleep();
				break;
			case STATE_ADJUSTCLOCK:
				if (tick_counter<=(MAX_TICK_ADJUSTCLOCK-2))
					click_silence();
				break;
			case STATE_ADJUSTALARM:
				if (tick_counter<=(MAX_TICK_ADJUSTALARM-2))
					click_silence();
				break;
			}
		}
	}
	if (event&EVENT_WAKEUP) {
		/* state will be STATE_SLEEP at this point */
		wakeup_from_sleep();
		switch_state_debounce();
	}
	if (event&EVENT_SLEEP) {
		/* state will be STATE_DEBOUNCE at this point */
		switch_state_sleep();
	}
}



int main(void)
{
	uint8_t c,o;
	/* 
	 */
	PCMSK|=_BV(ROTARY_PC_PIN_A);
	rotary_init();
	event_init();
	timer_init();
	buzzer_init();
	switch_state_sleep();
	for(;;) {
		mainloop();
	}
	return 0;
}

/*
 * Local Variables:
 * mode: c
 * c-file-style: "linux"
 * End:
 */

