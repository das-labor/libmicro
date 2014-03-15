#ifndef CANUART_H
#define CANUART_H

/*****************************************************************************
 * Send encapsulated CAN packets via UART
 *
 *
 */

#include "can.h"
#include "can-encap.h"
#include "uart.h"
#include "crc16.h"
#include "debug.h"

//the threshold that has to be exceeded to cause a resync procedure
//(if canu_failcnt > CANU_FAILTHRESH then resync)
#define CANU_FAILTHRESH 1
extern unsigned int canu_failcnt;

/*****************************************************************************
 * Connection management
 */

// Initialize CAN over UART on port serial
void canu_init(char *serial);

// syncronize line
void canu_reset(void);

void canu_close(void);

/*****************************************************************************
 * Memory Management
 */
rs232can_msg *canu_buffer_get(void);
void canu_free(rs232can_msg *);


/*****************************************************************************
 * rcv
 */

/* nonblocking read from uart -- returns 0 if no complete msg arrived */
rs232can_msg *canu_get_nb(void);

/* blocking read from uart */
rs232can_msg *canu_get(void);


/*****************************************************************************
 * transmit
 */
void canu_transmit(rs232can_msg *msg);
void canu_transmit_cmd(unsigned char cmd);

#endif
