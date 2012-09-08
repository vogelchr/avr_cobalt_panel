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
#include "hd44780.h"

#include <stdio.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

/* control pins, always output */
#define DDR_CTRL	DDRB
#define PORT_CTRL	PORTB
#define BIT_E	_BV(3)
#define BIT_RS	_BV(4)

/* shared data bus, spread over two avr ports */
#define PORT_DATA_1	PORTC
#define PIN_DATA_1      PINC
#define DDR_DATA_1	DDRC
#define MASK_DATA_1	0x3f  /* bit 0..5 */

#define PORT_DATA_2	PORTD
#define PIN_DATA_2      PIND
#define DDR_DATA_2	DDRD
#define MASK_DATA_2	0xc0  /* bit 6..7 */

#define HD44780_DATA   1  /* RS line */
#define HD44780_CMD    0

#define HD44780_CMD_CLR   0x01
#define HD44780_CMD_HOME  0x02

#define BIT(onoff,n) ((!!(onoff)<<(n)))

/* ID: 0=decrement cursor position, 1=increment cursor position */
/* S:  0=display fix                1=display moves */
#define HD44780_CMD_MOD(ID,S)    (0x04|BIT(ID,1)|BIT(S,0)          )

/* D:  0=display off                1=display on */
/* C:  0=cursor off                 1=cursor on */
/* B:  0=cursor does not blink      1=cursor blinks */
#define HD44780_CMD_DC(D,C,B)    (0x08|BIT(D,2) |BIT(C,1) |BIT(B,0))

/* SC=0 move cursor                 1=move display contents */
/* RL=0 move left                   1=move right */
#define HD44780_CMD_SHIFT(SC,RL) (0x10|BIT(SC,3)|BIT(RL,2)         )

/* DL=0 4-bit interface             1=8bit interface */
/* N=0  1 line display              N=1 two line display (or four) */
/* F=0  5x7                         F=1 5x10 font */
#define HD44780_CMD_FUN(DL,N,F)  (0x20|BIT(DL,4)|BIT(N,3) |BIT(F,2))

#define HD44780_CMD_CGRAM(n)     (0x40|((n)&0x1f))
#define HD44780_CMD_DDRAM(n)     (0x80|((n)&0x3f))

void
hd44780_dbus_write(uint8_t byte)
{
	/* put data on output pins, enable drivers */
	PORT_DATA_1 = (PORT_DATA_1 & ~ MASK_DATA_1) | (byte & MASK_DATA_1);
	PORT_DATA_2 = (PORT_DATA_2 & ~ MASK_DATA_2) | (byte & MASK_DATA_2);
	DDR_DATA_1 |= MASK_DATA_1;
	DDR_DATA_2 |= MASK_DATA_2;
}

uint8_t
hd44780_dbus_read()
{
	uint8_t ret;
	ret  = (PIN_DATA_1 & MASK_DATA_1);
	ret |= (PIN_DATA_2 & MASK_DATA_2);
	return ret;
}

void
hd44780_dbus_tristate()
{
	DDR_DATA_1 &= ~ MASK_DATA_1; /* disable drivers on data bus */
	DDR_DATA_2 &= ~ MASK_DATA_2;
	PORT_DATA_1 = (PORT_DATA_1 & ~ MASK_DATA_1); /* disable pullups */
	PORT_DATA_2 = (PORT_DATA_2 & ~ MASK_DATA_2);
}

static void hd44780_write(uint8_t byte,uint8_t rs)
{
	cli();
	if(rs)
		PORT_CTRL |= BIT_RS;
	else
		PORT_CTRL &= ~BIT_RS;
	hd44780_dbus_write(byte);
	_delay_us(1);  /* data setup time: 195 ns */
	PORT_CTRL |=   BIT_E; /* E -> high */
	_delay_us(1);  /* enable pulse width (high) min. 450 ns */
	PORT_CTRL &= ~ BIT_E; /* E -> low  */

	hd44780_dbus_tristate();

	sei();
}

void
hd44780_cmd(uint8_t byte)
{
//	printf("<%02hhx>",byte);
	hd44780_write(byte,HD44780_CMD);

	if(byte == 0x01 || (byte & 0xfe) == 0x02){
		_delay_us(3040); /* 1,52 ms (datasheet) */
	} else {
		_delay_us(74);   /* 37 us (datasheet) */
	}
}

void
hd44780_data(uint8_t byte)
{
//	printf("[%02hhx]",byte);
	hd44780_write(byte,HD44780_DATA);
	_delay_us(74); /* 37us (datasheet) */
}

void hd44780_data_PSTR(PGM_P data,uint8_t len)
{
	while(len--)
		hd44780_data(pgm_read_byte(data++));
}

void
hd44780_init()
{
	cli();
	DDR_CTRL  |=  (BIT_E | BIT_RS); /* E, RS output */
	PORT_CTRL &= ~(BIT_E | BIT_RS); /* E = RS = low */
	hd44780_dbus_tristate();
	sei();
}

void
hd44780_cursor(int x, int y, int cursor, int blink)
{
	uint8_t p = x + (y ? 40 : 0);
//	printf("{%d,%d,%d,%d -> %d}",x,y,cursor,blink,p);
	hd44780_cmd(HD44780_CMD_DC(1,cursor,blink));
	hd44780_cmd(HD44780_CMD_DDRAM(p));
}


void
hd44780_clear()
{
	hd44780_cmd(HD44780_CMD_CLR);
	hd44780_cmd(HD44780_CMD_HOME);
}

void
hd44780_home()
{
	hd44780_cmd(HD44780_CMD_HOME);
}

void
hd44780_reset()
{
	/* force 8-bit interface: */
	hd44780_cmd(0x33); /* special HD44780_CMD_FUN, two nibbles 0x03 */
	hd44780_cmd(0x33); /* special HD44780_CMD_FUN, two nibbles 0x03 */
	hd44780_cmd(HD44780_CMD_FUN(1,1,0)); /* 8bit, 2line, 5x7 font */

	hd44780_cmd(HD44780_CMD_MOD(1,0)); /* cursor moves left to right */
	hd44780_cmd(HD44780_CMD_DC(1,0,0)); /* display on, cursor off, blink off */
	hd44780_clear();
}

void
hd44780_cgram(uint8_t i){
	hd44780_cmd(HD44780_CMD_CGRAM(i));
}

