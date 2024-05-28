#include <stdlib.h>
#include <string.h>
#include "headers/client_t.h"
#include "../game/headers/player.h"

client_t *create_client(int sock) {
    client_t *res = malloc(sizeof(client_t));
    memset(res, 0, sizeof(client_t));

    res->sock_fd = sock;

    return res;
}