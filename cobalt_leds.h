#ifndef COBALT_LEDS
#define COBALT_LEDS

#include <stdint.h>

#define COBALT_LED_WEB   (1<<0)
#define COBALT_LED_DISK  (1<<1)
#define COBALT_LED_LOGO  (1<<2)

#define COBALT_LED_NET   (0xf0)
#define COBALT_LED_TXRX  (1<<4)  /* Network LEDs */
#define COBALT_LED_100M  (1<<5)
#define COBALT_LED_LINK  (1<<6)
#define COBALT_LED_COL   (1<<7)

/* initialize ports */
extern void cobalt_leds_init();

/* light up LEDs, 1=on 0=off */
extern void cobalt_leds_set(uint8_t leds);

#endif
