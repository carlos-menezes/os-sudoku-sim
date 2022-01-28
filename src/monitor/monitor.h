#ifndef MONITOR_H
#define MONITOR_H

#include "libcom.h"

int handle_handshake();
void handle_communication();
void spawn_threads();
void *init_game_for_thread(void *thread_id);
void handle_server_message(struct server_msg_t *in_msg);
void cleanup();

#endif