#include "protocol_header.h"
#include <stdio.h>
#include <arpa/inet.h> 


void set_bit(char *buf, int pos) {
    buf[pos / 8] |= (1 << (7 - (pos % 8)));
}

int get_bit(const char *buf, int pos) {
    return (buf[pos / 8] >> (7 - (pos % 8))) & 1;
}


void encode_protocol_header(protocol_header *header, uint16_t codereq, uint8_t id, uint8_t eq) {
     // Clear the buffer
    for (int i = 0; i < 16; i++) {
        header->data[i] = 0;
    }

    // Encode codereq
    for (int i = 0; i < CODEREQ_SIZE; i++) {
        if ((codereq >> (CODEREQ_SIZE - 1 - i)) & 1) {
            set_bit(header->data, CODEREQ_POS + i);
        }
    }

    // Encode id
    for (int i = 0; i < ID_SIZE; i++) {
        if ((id >> (ID_SIZE - 1 - i)) & 1) {
            set_bit(header->data, ID_POS + i);
        }
    }

    // Encode eq
    if (eq) {
        set_bit(header->data, EQ_POS);
    }
}

void decode_protocol_header(const protocol_header *header, uint16_t *codereq, uint8_t *id, uint8_t *eq) {
    for (int i = 0; i < CODEREQ_SIZE; i++) {
        *codereq |= (get_bit(header->data, CODEREQ_POS + i) << (CODEREQ_SIZE - 1 - i));
    }

    for (int i = 0; i < ID_SIZE; i++) {
        *id |= (get_bit(header->data, ID_POS + i) << (ID_SIZE - 1 - i));
    }

    *eq = get_bit(header->data, EQ_POS);
}


