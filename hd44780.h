#ifndef HD44780_H
#define HD44780_H

#include <avr/pgmspace.h>
#include <stdint.h>

/* talk to a hd44780 lcd connected to an avr mega168
   according to the connection table below: */

/* LCD   Port   Arduino
   D0 -> PC0    Analog 0
   ...
   D5 -> PC5    Analog 5
   D6 -> PD6    Digital 6
   D7 -> PD7    Digital 7

   E  -> PB3    Digital 11
   RS -> PB4    Digital 12
   RW -> GND    (always GND -> only write)
*/

/* initialize hardware (ports, data direction register...) */
extern void hd44780_init();

/* perform display clear, cursor home, initialization of the controller */
extern void hd44780_reset();

/* move cursor to x(0..15),y(0..1)  turn on/off cursor and/or blink */
extern void hd44780_cursor(int x, int y, int cursor, int blink);

/* clear display, move cursor to home */
extern void hd44780_clear();

/* move cursor home */
extern void hd44780_home();

/* write command to the LCD, delay added according to the command */
extern void hd44780_cmd(uint8_t byte);

/* write data to the LCD */
extern void hd44780_data(uint8_t byte);

/* write data from PROGMEM to LCD */
extern void hd44780_data_PSTR(PGM_P data,uint8_t len);

/* the databus is shared with another peripheral, hence these
   functions can be used to put any byte on the data bus,
   read from it, or set it to tristate */

/* put byte on D0..D7, enable drivers */
extern void hd44780_dbus_write(uint8_t byte);

/* read byte from D0..D7 */
extern uint8_t hd44780_dbus_read();

/* put D0..D7 into high-Z mode */
extern void hd44780_dbus_tristate();

/* set data pointer into character generator ram */
extern void hd44780_cgram(uint8_t i);


#endif
