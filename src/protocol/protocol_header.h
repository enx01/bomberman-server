#include <stdint.h>
#ifndef PROTOCOL_HEADER_H
#define PROTOCOL_HEADER_H


#define CODEREQ_SIZE 13
#define ID_SIZE 2
#define EQ_SIZE 1

#define CODEREQ_POS 0
#define ID_POS 13
#define EQ_POS 15

struct protocol_header {
    char data[16];
};
typedef struct protocol_header protocol_header;

void encode_protocol_header(protocol_header *header, uint16_t codereq, uint8_t id, uint8_t eq);
void decode_protocol_header(const protocol_header *header, uint16_t *codereq, uint8_t *id, uint8_t *eq);



#endif // !PROTOCOL_HEADER_H