#ifndef SERIAL_H
#define SERIAL_H

/* return non-zero when there's a char to be read from stdin */
extern int
serial_status();

/* initialize UART, setup interrupts, connect stdin/stdout to serial port. */
extern void
serial_init();

#endif
