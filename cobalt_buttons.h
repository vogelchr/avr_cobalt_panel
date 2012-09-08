#ifndef COBALT_BUTTONS_H
#define COBALT_BUTTONS_H

#include <stdint.h>

/* see README for connection of buttons to port pins */

#define COBALT_BUTTON_PIN   (1<<1) /* hidden beneath hole right of display */
#define COBALT_BUTTON_LEFT  (1<<2)
#define COBALT_BUTTON_UP    (1<<3)
#define COBALT_BUTTON_DOWN  (1<<4)
#define COBALT_BUTTON_RIGHT (1<<5)
#define COBALT_BUTTON_E     (1<<6)
#define COBALT_BUTTON_S     (1<<7)

#define COBALT_BUTTONS_ALL  0xfe

/* setup data direction registers etc... */
extern void cobalt_buttons_init();

/* get bitfield of currently pressed buttons */
extern uint8_t cobalt_buttons_get();

/* debounce buttons:

   This function is called repeatedly with a constant "debounce counter"
   argument. The currently pressed buttons are compared with the state
   of buttons the last time this function has been called.

   If the button inputs have been on a stable value for "debounce_counter"
   invocations of this function, then it will return 1 *once*.
*/
   
extern uint8_t cobalt_buttons_debounce(uint8_t *buttons,uint8_t debounce_counter);

/* convert the button mask buttons to a single char
   LEFT -> 'L', RIGHT -> 'R',
   UP -> 'U',   DOWN  -> 'D',
   E -> 'E',    S -> 'S',
   PIN -> 'P'
*/

extern char cobalt_buttons_simple_char(uint8_t buttons);

#endif
