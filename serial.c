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
#include "serial.h"
#include <stdio.h>
#include <avr/interrupt.h>

/* static FILE structure for stdout */
static int uart_putchar(char c, FILE *stream);
static int uart_getchar(FILE *stream);

static FILE uart_stdout = FDEV_SETUP_STREAM(uart_putchar, NULL, _FDEV_SETUP_WRITE);
static FILE uart_stdin  = FDEV_SETUP_STREAM(NULL, uart_getchar, _FDEV_SETUP_READ);

#define UART_BUFSIZE    32

static char uart_outbuf[UART_BUFSIZE];
static uint8_t uart_writep;
static uint8_t uart_readp;

ISR(USART_UDRE_vect){
	uint8_t rp = uart_readp;
	UDR0 = uart_outbuf[rp];

	rp = (rp + 1)%UART_BUFSIZE;
	uart_readp = rp;

	if(rp == uart_writep){ /* empty */
		UCSR0B &= ~ _BV(UDRIE0); /* turn off Data Register Empty Interrupt */
	}
}

static inline void
uart_put_into_outbuf(unsigned char c){
	uint8_t wp;

	while(1){
		cli();
		wp = uart_writep;
		uart_outbuf[wp]=c;
		wp = (wp + 1)%UART_BUFSIZE;
		if(wp == uart_readp){ /* full! */
			sei();
			continue;
		}
		uart_writep = wp;
		if(!(UCSR0B & _BV(UDRIE0))) /* if Data Register Empty Interrupt... */
			UCSR0B |= _BV(UDRIE0); /* is off, turn it on. */
		sei();
		break;
	}
}

static int
uart_putchar(char c, FILE *stream)
{
	if(c == '\r')
		return 0;
	if(c == '\n')
		uart_put_into_outbuf('\r');
	uart_put_into_outbuf(c);
	return 0;
}

static int
uart_getchar(FILE *stream)
{
	/* wait for Receive Complete bit to be set */
//	while(!(UCSR0A & _BV(RXC0)));
	return UDR0;
}

/* there is a byte to be read */
int
serial_status(){
	return UCSR0A & _BV(RXC0);
}

void
serial_init(){
	UBRR0  = 103; /* 16 MHz clockrate, U2Xn=0, 9600 bps */
	UCSR0B = _BV(RXEN0)|_BV(TXEN0);
	UCSR0C = _BV(UCSZ01)|_BV(UCSZ00); /* 8 bit, no parity, 1 stopbit */

	stdout = &uart_stdout;
	stdin  = &uart_stdin;
}
