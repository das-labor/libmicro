/*
 * This implements the CRC16 algorithm used in AVR-Libc.
 * http://www.nongnu.org/avr-libc/user-manual/group__util__crc.html
 */

unsigned int crc16_update(unsigned int crc, unsigned char a)
{
        unsigned int i;

        crc ^= a;
        for (i = 0; i < 8; ++i) {
                if (crc & 1)
                        crc = (crc >> 1) ^ 0xA001;
                else
                        crc = (crc >> 1);
        }

        return crc & 0xFFFF;
}

unsigned int crc16(unsigned char *buf, unsigned int len)
{
        unsigned int i, crc;

        for (i = 0, crc = 0; i < len; i++)
                crc = crc16_update(crc, *buf++);

        return crc;
}
