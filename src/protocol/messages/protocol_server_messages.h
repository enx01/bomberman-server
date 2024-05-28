#ifndef SERVER_MESSAGES_H
#define SERVER_MESSAGES_H

#include <stdint.h>
#include "../protocol_header.h"

// IGM
struct init_game_msg {
    protocol_header header;
    uint16_t portudp,
             portmdiff;
    char adrmdiff[128];
    /* 
     *  +-0-+-1-+-2-+-3-+-4-+-5-+-6-+-7-+-8-+-9-+-0-+-1-+-2-+-3-+-4-+-5-+
     *  |CODEREQ                                            |ID     |EQ | header (B.E.) 
     *  +---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+
     *  |PORTUDP							                            | portudp (B.E.)
     *  +---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+
     * 	|PORTMDIFF							                            | portmdiff (B.E.)
     * 	+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+
     *  |ADRMDIFF ... (128 bytes)                                         adrmdiff (B.E.)
     *  +---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+
     *   
     *  - CODEREQ :
     *      - 9 if FFA;
     *      - 10 if TDM.
     *  - ID : [0::3].
     *  - EQ : 0 || 1.
     *  - PORTUDP : udp port number on which server awaits for incomming player msgs;
     *  - PORTMDIFF : port on which server will broadcast its messages to players;
     *  - ADRMDIFF : ipv6 address to which players must subscribe.
     *
     * */
};
typedef struct init_game_msg init_game_msg;

// GGM
struct game_grid_msg {
    protocol_header header;
    uint16_t  num;
    uint8_t height,
            width;
    uint8_t *data;
    /* 
     * Message broadcasted every seconds :
     *
     *  +-0-+-1-+-2-+-3-+-4-+-5-+-6-+-7-+-8-+-9-+-0-+-1-+-2-+-3-+-4-+-5-+
     *  |CODEREQ                                            |ID     |EQ | header (B.E.) 
     *  +---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+
     *  |NUM    							                            | num (B.E.)
     *  +---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+
     * 	|HEIGHT                         |WIDTH                          | height, width
     * 	+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+
     *  |TILE0                          |TILE1 ...                        data
     *  +---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+
     *
     *  - CODEREQ : 11;
     *  - ID & EQ : 0;
     *  - NUM : message number % 2^16 (messages sent every seconds);
     *  - HEIGHT & WIDTH : dimensions of the grid;
     *  - data : each tile is encoded on 8 bytes :
     *      - 0 if EMPTY;
     *      - 1 if WALL;
     *      - 2 if D_WALL;
     *      - 3 if BOMB;
     *      - 4 if E_BOMB;
     *      - 5 + i if PLAYER with player.id == i.
     *
     * Message broadcasted every `freq` ms :
     * 
     *  +-0-+-1-+-2-+-3-+-4-+-5-+-6-+-7-+-8-+-9-+-0-+-1-+-2-+-3-+-4-+-5-+
     *  |CODEREQ                                            |ID     |EQ | header (B.E.) 
     *  +---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+
     *  |NUM    							                            | num (B.E.)
     *  +---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+
     * 	|NB                             |0x00                           | height
     * 	+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+
     *  |TILE0                                                            data
     * 	+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+
     * 	|                               |TILE1...
     * 	+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+
     * 	
     * 	- CODEREQ : 12;
     * 	- ID & EQ : 0;
     *  - NUM : message number % 2^16 (messages sent every `freq` ms);
     *  - NB : number of transmitted tiles.
     *  - data : each tile is encoded on 24 bytes (3 octets) :
     *      - 1st octet : tile's line number;
     *      - 2nd octet : tile's column number;
     *      - 3rd octet : tile's content.
     * 
     *  */
     
};
typedef struct game_grid_msg game_grid_msg;


// SCM
struct server_chat_msg {
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
     *      - 13 if GLOBAL_MSG;
     *      - 14 if TEAM_MSG;
     *  - ID : sender's id.
     *  - EQ : sender's team id;
     *  - LEN : data length.
     *  - DATA : message data.
     *
     * */
};
typedef struct server_chat_msg server_chat_msg;


// EGM
struct end_game_msg {
    protocol_header header;
    /*  +-0-+-1-+-2-+-3-+-4-+-5-+-6-+-7-+-8-+-9-+-0-+-1-+-2-+-3-+-4-+-5-+
     *  |CODEREQ                                            |ID     |EQ | header (B.E.)
     *  +---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+
     *
     *  - CODEREQ :
     *      - 15 if FFA;
     *      - 16 if TDM.
     *  - ID :
     *      - winner's id if CODEREQ == FFA;
     *      - else, ignored.
     *  - EQ : 
     *      - victorious team id if CODEREQ == TDM;
     *      - else, ignored.
     * */
};
typedef struct end_game_msg end_game_msg;


//void decode_end_game_msg(end_game_msg *msg, uint16_t *codereq, uint8_t *id, uint8_t *eq);
void encode_end_game_msg(end_game_msg *msg, uint16_t codereq, uint8_t id, uint8_t eq);
void decode_end_game_msg(char *buffer, uint16_t *codereq, uint8_t *id, uint8_t *eq);
void decode_server_chat_msg(char *buffer, protocol_header *pth, uint8_t *msg_len, char *data);


#endif // !SERVER_MESSAGES_H