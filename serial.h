/**
 * serial.h - function definitions see serial.s
 */

/**
 * void serial_setup() - set port and bit duration
 */
void serial_setup(unsigned mask, unsigned duration);

/**
 * void putc() - transmit one character with start and stop bits
 */
void putc(int);

/**
 * void puts() - transmit a null terminated array of characters
 */
//int puts(const char *);

