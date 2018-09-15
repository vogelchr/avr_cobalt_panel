/* ----------------------------------------------------------------------------
This file is part of avr_cobalt_panel.

(c) 2012 Christian Vogel <vogelchr@vogel.cx>

avr_cobalt_panel is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

avr_cobalt_panel is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

For a copy of the GNU General Public License see http://www.gnu.org/licenses/.
---------------------------------------------------------------------------- */

#include "cobalt_buttons.h"
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <avr/pgmspace.h>

#include "hd44780.h"

#define PORT_CTRL    PORTB
#define DDR_CTRL     DDRB
#define BIT_nOE      _BV(5)


void
cobalt_buttons_init()
{
	PORT_CTRL |= BIT_nOE;
	DDR_CTRL  |= BIT_nOE;
}

static uint8_t cobalt_buttons_last=0; /* last read button value */
static uint8_t cobalt_buttons_ctr=0;  /* debounce timer */

uint8_t
cobalt_buttons_debounce(uint8_t *buttons,uint8_t debounce_counter)
{
	uint8_t b = cobalt_buttons_get();

	/* different than last time? Set debounce timer to given value. */
	if(b != cobalt_buttons_last){
		cobalt_buttons_last = b; /* update last read button value */
		cobalt_buttons_ctr = debounce_counter;
		return 0;
	}

	if(cobalt_buttons_ctr == 0)
		return 0; /* debounce counter has already expired, no changes */

	cobalt_buttons_ctr--;

	if(cobalt_buttons_ctr > 0)
		return 0; /* debounce counter has not yet expired */

	/* debounce counter has expired, return button to user */
	*buttons = b;
	return 1;
}

uint8_t
cobalt_buttons_get()
{
	uint8_t ret;

	cli();
	PORT_CTRL &= ~BIT_nOE;
	_delay_us(2);
	ret = hd44780_dbus_read();
	PORT_CTRL |= BIT_nOE;
	sei();

	ret ^= 0xff;
	return ret & COBALT_BUTTONS_ALL;
}

const char button_chars[8] PROGMEM = "0PLUDRES";

char
cobalt_buttons_simple_char(uint8_t buttons){
	uint8_t k;
	const char * p = button_chars;

	for(k=0;k<8;k++){
		if(buttons == (1<<k))
			return pgm_read_byte(p);
		p++;
	}
	return '\0';
}
