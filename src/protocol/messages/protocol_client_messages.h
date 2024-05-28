#ifndef CLIENT_MESSAGES_H
#define CLIENT_MESSAGES_H

#include <stdint.h>
#include <string.h>

#include "../protocol_header.h"

// JGM
struct join_game_msg {
    protocol_header header;
    /* 
     *  +-0-+-1-+-2-+-3-+-4-+-5-+-6-+-7-+-8-+-9-+-0-+-1-+-2-+-3-+-4-+-5-+
     *  |CODEREQ                                            |ID     |EQ | header (B.E.)
     *  +---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+
     *   
     *   if player -> joining :
     *      - CODEREQ : 
     *          - 1 if FFA;
     *          - 2 if TDM.
     *      - ID : 0 (ignored).
     *      - EQ : 0 (ignored).
     *   if player -> ready :
     *      - CODEREQ :
     *          - 3 if FFA;
     *          - 4 if TDM.
     *      - ID : [0::3] (player id).
     *      - EQ : 0 || 1;
     *
     * */
};

// ADM
struct action_data_msg {
    protocol_header header;
    uint16_t data;
    /* 
     *  +-0-+-1-+-2-+-3-+-4-+-5-+-6-+-7-+-8-+-9-+-0-+-1-+-2-+-3-+-4-+-5-+
     *  |CODEREQ                                            |ID     |EQ | header (B.E.)
     *  +---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+
     *  |NUM                                                |ACTION     | data (B.E.)
     *  +---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+
     *  
     *   - CODEREQ :
     *      - 5 if FFA;
     *      - 6 if TDM.
     *   - ID : sender's id.
     *   - EQ : sender's team id (later check if CODEREQ == 6);
     *   - NUM : message number % 2^13
     *   - ACTION :
     *      - 0 if NORTH;
     *      - 1 if EAST;
     *      - 2 if SOUTH;
     *      - 3 if WEST;
     *      - 4 if BOMB;
     *      - 5 if CANCEL_LAST_REQUEST.
     *
     * /!\ Warning if NUM == 8191, next will be 0. /!\
     *
     * */
};
typedef struct action_data_msg action_data_msg;

// CCM
struct client_chat_msg {
    protocol_header header;
    uint8_t msg_len;
    char *data;
    /* 
     *  +-0-+-1-+-2-+-3-+-4-+-5-+-6-+-7-+-8-+-9-+-0-+-1-+-2-+-3-+-4-+-5-+
     *  |CODEREQ                                            |ID     |EQ | header (B.E.)
     *  +---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+
     *  |LEN                            |DATA...                          msg_len + data
     *  +---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+
     *
     *  - CODEREQ :
     *      - 7 if GLOBAL_MSG;
     *      - 8 if TEAM_MSG;
     *  - ID : sender's id.
     *  - EQ : sender's team id;
     *  - LEN : data length.
     *  - DATA : message data.
     *
     * */

};
typedef struct client_chat_msg client_chat_msg;


void encode_action_data_msg(struct action_data_msg *msg, uint16_t codereq, uint8_t id, uint8_t eq, uint16_t num, uint8_t action);
void decode_action_data_msg(struct action_data_msg *msg, uint16_t *codereq, uint8_t *id, uint8_t *eq, uint16_t *num, uint8_t *action);

#endif // !CLIENT_MESSAGES_H
