#ifndef MONITOR_H
#define MONITOR_H

void handle_communication();
void spawn_threads();
void* init_game_for_thread(void* thread_id);

#endif