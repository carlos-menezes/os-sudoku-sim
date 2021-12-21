#ifndef SERVER_H
#define SERVER_H

#include "libgrid.h"
#include "libarray.h"
#include "libconfigmonitor.h"

#define DISPATCH_TRIGGER_TIME 1000000 // 1 SEC
#define SCHEDULER_TRIGGER_TIME 3000000 // 1 SEC

#define INITIAL_PRIORITY 5

typedef enum
{
    STATE_WAITING_INIT,
    STATE_GUESSING,
    STATE_DEAD
} STATE_PHASE;


struct monitor_state_t {
    char monitor[MAX_MONITOR_NAME];
    unsigned int priority;
    unsigned int socket_fd;
    STATE_PHASE phase;
};

struct game_state_t {
    struct grid_t grid;
    struct linked_list_t* monitor_states;
    struct linked_list_t* monitor_handlers;
    pthread_mutex_t cell_mutex[GRID_SIZE];
};

void init_game_state(struct game_state_t *state);
void handle_communication();
void* dispatch();
void* scheduler();
void* allow_init();
void* handle_monitor();

#endif