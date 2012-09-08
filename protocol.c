#include <stdio.h>

#include "protocol.h"
#include "cobalt_leds.h"
#include "hd44780.h"

/* serial protocol:

   any character except ESC -> send to LCD

   *  ESC 'c'      -> _C_lear and home LCD
   *  ESC 'h'      -> _H_ome LCD

   *  p -> cursor _P_osition
      ESC 'p' x y c b  -> goto : x={'0'..'9','a'..'f'}, y={'0','1'} -> goto x y
                        c = show cursor ('0' or '1'), b= blink ('0' or '1')

   *  l -> _L_EDS
      ESC 'l' x y  -> set LEDs, x+y = hexadecimal LED mask

   * u -> _U_ser defined character

      ESC 'u' x a b c d e f g h  -> define user character at position
                               u (hex nibble 0..7)
			       with binary characters a .. h 
*/

static uint8_t hex_nibble(char c){
	if(c >= '0' && c <= '9')
		return c - '0';
	if(c >= 'A' && c <= 'F')
		return c - 'A' + 10;
	if(c >= 'a' && c <= 'f')
		return c - 'a' + 10;
	return 0;
}

static uint8_t hex_byte(char a, char b){
	return (hex_nibble(a)<<4)|hex_nibble(b);
}

enum parser_state {
	parser_idle,
	parser_esc,
	parser_feed
};

int parser_buf[9];
int parser_eat_n;
char parser_esc_char;

enum parser_state parser_state;

void
protocol_eat_char(char c)
{
	switch(parser_state){
	case parser_idle:
		if(c == '\e'){
			parser_state = parser_esc;
		} else {
			hd44780_data(c); /* write out character to LCD */
		}
		break;
	case parser_esc:
		parser_esc_char = c;
		switch(c){
		case 'c':
			hd44780_clear(c);
			parser_state = parser_idle;
			break;
		case 'h':
			hd44780_home(c);
			parser_state = parser_idle;
			break;
		case 'p':
			parser_eat_n=4;
			parser_state = parser_feed;
			break;
		case 'l':
			parser_eat_n=2;
			parser_state = parser_feed;
			break;
		case 'u':
			parser_eat_n=9;
			parser_state = parser_feed;
			break;
		}
		break;
	case parser_feed:
		if(parser_eat_n){
			parser_buf[--parser_eat_n]=c;
			if(parser_eat_n)
				break;
		}
		parser_state = parser_idle;

		switch(parser_esc_char){
		case 'p':
			hd44780_cursor(
				hex_nibble(parser_buf[3]),
				hex_nibble(parser_buf[2]),
				hex_nibble(parser_buf[1]),
				hex_nibble(parser_buf[0]));
			break;
		case 'l':
			cobalt_leds_set(
				hex_byte(parser_buf[1],
					parser_buf[0]));
			break;
		case 'u':
			c = hex_nibble(parser_buf[8]);
			hd44780_cgram(c*8);
			for(c=0;c<8;c++)
				hd44780_data(parser_buf[7-c]);
			hd44780_cursor(0,0,0,0);
			break;
		}
		break;
	}
}
