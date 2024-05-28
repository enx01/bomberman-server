#include <stdint.h>
#include <string.h>
#include <arpa/inet.h>
#include "protocol_server_messages.h"
#include "protocol_client_messages.h"


// void decode_end_game_msg(end_game_msg *msg, uint16_t *codereq, uint8_t *id, uint8_t *eq) {
//     decode_protocol_header(&msg->header, codereq, id, eq);
// }


void encode_end_game_msg(end_game_msg *msg, uint16_t codereq, uint8_t id, uint8_t eq) {
    encode_protocol_header(&msg->header, codereq, id, eq);
}


void decode_end_game_msg(char *buffer, uint16_t *codereq, uint8_t *id, uint8_t *eq) {
    memcpy(codereq, buffer, sizeof(uint16_t));
    *codereq = ntohs(*codereq);
    memcpy(id, buffer + sizeof(uint16_t), sizeof(uint8_t));
    memcpy(eq, buffer + sizeof(uint16_t) + sizeof(uint8_t), sizeof(uint8_t));
}

void decode_server_chat_msg(char *buffer, protocol_header *pth, uint8_t *msg_len, char *data){
    memcpy(pth, buffer, sizeof(protocol_header));
    memcpy(msg_len, buffer + sizeof(protocol_header), sizeof(uint8_t));
    memcpy(data, buffer + sizeof(protocol_header) + sizeof(uint8_t), *msg_len);
}