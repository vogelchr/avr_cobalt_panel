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

#include <avr/io.h>
#include <avr/pgmspace.h>
#include <stdio.h>
#include <util/delay.h>

#include "hd44780.h"
#include "cobalt_leds.h"
#include "cobalt_buttons.h"
#include "serial.h"
#include "menu.h"
#include "protocol.h"

const char hello_one[] PROGMEM = "avr-cobalt-panel";
const char hello_two[] PROGMEM = "github/vogelchr";

int
main(void)
{
	uint8_t button;
	uint16_t i;

	serial_init();

	hd44780_init();
	cobalt_leds_init();
	cobalt_buttons_init();

	hd44780_reset();
	hd44780_data_PSTR(hello_one,16);
	hd44780_cursor(0,1,1,0); /* x,y,cursor,blink */
	hd44780_data_PSTR(hello_two,15);

	while(1){
		i++;
		_delay_us(10);

		if(serial_status())
			protocol_eat_char(getchar());

		if(cobalt_buttons_debounce(&button,200)){
			char c = cobalt_buttons_simple_char(button);
			/* we are only interested in single key presses,
			   not combinations */
			
			if(c)
				putchar(c);
		}
	}
}
