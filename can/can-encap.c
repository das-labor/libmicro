
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <string.h>
#include <libmicro/can-encap.h>

void can_message_from_can_message_raw(can_message *cmsg, can_message_raw *rmsg)
{
        cmsg->addr_src = (uint8_t) (rmsg->id >> 8);
        cmsg->addr_dst = (uint8_t) (rmsg->id);
        cmsg->port_src = (uint8_t) ((rmsg->id >> 23) & 0x3f);
        cmsg->port_dst = (uint8_t) (((rmsg->id >> 16) & 0x0f) |
                                    ((rmsg->id >> 17) & 0x30));
        cmsg->dlc = rmsg->dlc;
        memcpy(cmsg->data, rmsg->data, rmsg->dlc);
}

void can_message_raw_from_can_message(can_message_raw *raw_msg, can_message *cmsg)
{
        memset(raw_msg, 0, sizeof(can_message_raw));

        raw_msg->id = ((cmsg->port_src & 0x3f) << 23) |
                      ((cmsg->port_dst & 0x30) << 17) |
                      ((cmsg->port_dst & 0x0f) << 16) |
                      (cmsg->addr_src << 8) | (cmsg->addr_dst);
        raw_msg->dlc = cmsg->dlc;
        memcpy(raw_msg->data, cmsg->data, cmsg->dlc);
}

void can_message_raw_from_rs232can_msg(can_message_raw *cmsg, rs232can_msg *rmsg)
{
        memcpy(cmsg, rmsg->data, sizeof(can_message_raw) );
}


void rs232can_msg_from_can_message_raw(rs232can_msg *rmsg, can_message_raw *cmsg)
{
        rmsg->cmd = RS232CAN_PKT;
        rmsg->len = sizeof(can_message_raw) + cmsg->dlc - 8;
}