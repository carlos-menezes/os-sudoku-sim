#ifndef SERVER_H
#define SERVER_H

#include "libarray.h"
#include "libconfigmonitor.h"
#include "libgrid.h"

#define DISPATCH_TRIGGER_TIME 500000 * 2 * 2 // 2s

#define BASE_PRIORITY 5

struct monitor_state_t
{
    char monitor[MAX_MONITOR_NAME];
    unsigned int socket_fd;

    // Stats
    unsigned int guesses;
    unsigned int correct_guesses;

    unsigned int priority;
    unsigned int last_checked;
};
struct game_state_t
{
    struct grid_t grid;
    struct node_t *states;   // linked_list_t<monitor_state_t*>*
    struct node_t *handlers; // linked_list_t<pthread_t*>*
    pthread_mutex_t cell_mutex[GRID_SIZE];
};

void init_game_state(struct game_state_t *state);
void handle_communication();
void *dispatch();
void *init_dispatch();
void *handle_monitor(void *socket_fd);
void *handle_monitor_message(void *in_msg);
void cleanup();

#endif