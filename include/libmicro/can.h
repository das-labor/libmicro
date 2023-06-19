#ifndef _CAN_H
#define _CAN_H

/*****************************************************************************
 * Simple CAN Library
 *
 * #define CAN_INTERRUPT 1      //set this to enable interrupt driven
 *                                and buffering version
 * #define CAN_RX_BUFFER_SIZE 2 //only used for Interrupt
 * #define CAN_TX_BUFFER_SIZE 2 //only used for Interrupt
 */

#define CAN_MESSAGE_RAW_HEADER_SIZE (4+1)

#define CAN_MAX_DATA_BYTES (8)

/*****************************************************************************
 * Types
 */

#include <stdint.h>
#include <unistd.h>

typedef struct rs232can_msg rs232can_msg;

typedef unsigned char can_addr;
typedef unsigned char can_port;
typedef uint16_t can_channel_t;
typedef uint8_t can_subchannel_t;

// Note: DLC is CAN terminology for Data Length Code
// It is a length field specifying the amount of data bytes in a frame

// "low level" representation
// id is a raw CAN ID
// see can-encap.h for encapsulation (TCP, rs232)
typedef struct {
        uint32_t id;
        uint8_t dlc;
        uint8_t data[CAN_MAX_DATA_BYTES];
} can_message_raw;

// "high level" representation
// this must be converted to a can_message_raw before sending it somewhere
typedef struct {
        can_addr      addr_src;
        can_addr      addr_dst;
        can_port      port_src;
        can_port      port_dst;
        unsigned char dlc;
        unsigned char data[CAN_MAX_DATA_BYTES];
} can_message;

// "high level" v2
// never really got established...
// must also be converted to a can_message_raw before sending it somewhere
typedef struct {
        can_channel_t    channel;
        can_subchannel_t subchannel;
        can_addr         addr_src;
        can_addr         addr_dst;
        uint8_t          dlc;
        uint8_t          data[CAN_MAX_DATA_BYTES];
} can_message_v2;


typedef enum { normal, mode_sleep, loopback, listenonly, config } can_mode_t ;


/*****************************************************************************
 * Global variables
 */
#ifdef CAN_HANDLEERROR
extern unsigned char can_error;
#endif


/*****************************************************************************
 * Management
 */

void can_init(void);
void can_setfilter(void);
void can_setmode(can_mode_t);
void can_setled(unsigned char led, unsigned char state);


/*****************************************************************************
 * Sending
 */

can_message *can_buffer_get(void);
void can_transmit(can_message *msg);

#ifdef _POSIX_VERSION
void can_transmit_raw_gateway_message(rs232can_msg *rmsg);
#endif


/*****************************************************************************
 * Receiving
 */

can_message *can_get(void);
can_message *can_get_nb(void);
void can_free(can_message *msg);

// this is only needed for Interrupt driven Version
#ifndef CAN_INTERRUPT
// #  define can_free(m)
void can_free(can_message *msg);
#else
void can_free(can_message *msg);
#endif


/*****************************************************************************
 * Sending
 */

can_message_raw *can_buffer_get_raw(void);
void can_transmit_raw(can_message_raw *msg);


/*****************************************************************************
 * Receiving
 */

can_message_raw *can_get_raw(void);
can_message_raw *can_get_raw_nb(void);

// this is only needed for Interrupt driven Version
#ifndef CAN_INTERRUPT
// #  define can_free(m)
void can_free_raw(can_message_raw *msg);
#else
void can_free_raw(can_message_raw *msg);
#endif

#ifdef _POSIX_VERSION
rs232can_msg * can_get_raw_gateway_message_nb(void);
#endif

/*****************************************************************************
 * Sending
 */

void can_transmit_v2(can_message_v2 *msg);


/*****************************************************************************
 * Receiving
 */

can_message_v2 *can_get_v2_nb(void);

void can_free_v2(can_message_v2 *msg);



#endif // _CAN_H

