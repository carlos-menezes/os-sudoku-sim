#ifndef SERVER_H
#define SERVER_H

#include "libgrid.h"
#include "libarray.h"
#include "libconfigmonitor.h"

#define DISPATCH_TRIGGER_TIME 500000 * 2 * 2 // 2s

#define BASE_PRIORITY 5

struct monitor_state_t {
    char monitor[MAX_MONITOR_NAME];
    unsigned int priority;
    unsigned int socket_fd;
};
struct game_state_t {
    struct grid_t grid;
    struct node_t* states; // linked_list_t<monitor_state_t*>*
    struct node_t* handlers; // linked_list_t<pthread_t*>*
    pthread_mutex_t cell_mutex[GRID_SIZE];
};

void init_game_state(struct game_state_t *state);
void handle_communication();
void* dispatch();
void*init_dispatch();
void* handle_monitor(void *socket_fd);

#endif