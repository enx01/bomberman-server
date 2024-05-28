#include "protocol_client_messages.h"
#include <stdio.h>
#include <arpa/inet.h> 


void encode_action_data_msg(struct action_data_msg *msg, uint16_t codereq, uint8_t id, uint8_t eq, uint16_t num, uint8_t action) {
    encode_protocol_header(&msg->header, codereq, id, eq);
    msg->data = (num << 3) | action;
}


void decode_action_data_msg(struct action_data_msg *msg, uint16_t *codereq, uint8_t *id, uint8_t *eq, uint16_t *num, uint8_t *action) {
    decode_protocol_header(&msg->header, codereq, id, eq);
    *num = (msg->data >> 3) & 0x1FFF;
    *action = msg->data & 0x07;
}


