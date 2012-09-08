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
#include "cobalt_leds.h"

#include <stdio.h>
#include <avr/io.h>
#include <avr/interrupt.h>

void cobalt_leds_init(){
	cli();
	DDRB  |=   0x07;  /* PB0 = Web, PB1 = Disk, PB2 = Logo */
	DDRD  |=   0x3c;    /* PD2..PD5 */
	sei();

	cobalt_leds_set(0);
}

void cobalt_leds_set(uint8_t leds){

	printf("Set LEDs 0x%02hhx.\n",leds);

	leds ^= (COBALT_LED_DISK | COBALT_LED_NET);

	cli();
	/* Web, Disk, Logo on portb, not inverted */
	PORTB = (PORTB & ~ 0x07) | ( leds & 0x07);
	/* upper 4 bits leds correspond to bits D2..D5 */
	PORTD = (PORTD & ~ 0x3c) | ( 0x3c & (leds >> 2) );
	sei();
}
