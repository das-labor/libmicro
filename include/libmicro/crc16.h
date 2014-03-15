#ifndef CRC16_H
#define CRC16_H

/*
 * This implements the CRC16 algorithm used in AVR-Libc.
 * http://www.nongnu.org/avr-libc/user-manual/group__util__crc.html
 */

// update a crc16 sum with the next byte a and return in
unsigned int crc16_update(unsigned int crc, unsigned char a);

// return the crc16 over a complete buffer of len bytes
unsigned int crc16(unsigned char *buf, unsigned int len);

#endif /* CRC16_H */
