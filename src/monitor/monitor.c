#include <time.h>
#include <stdlib.h>

#include "monitor.h"

#include "libio.h"
#include "liblog.h"
#include "libconfigmonitor.h"
#include "libarray.h"

monitor_t* monitor;
int guesses[GUESSES_SIZE];

void* init_game_for_thread(void* thread_id) {
    int id = (int*)thread_id;
    log_info(monitor->log_file, "INIT GAME | THREAD=%d", id);
    while (1)
    {
        log_info(monitor->log_file, "SLEEP | THREAD=%d", id);
        usleep(1000000);
    }
    pthread_exit(NULL);
}

void spawn_players() {
    monitor->players = malloc(sizeof(pthread_t) * monitor->config->players);
    for (size_t i = 0; i < monitor->config->players; i++)
    {
        if (pthread_create(monitor->players + i, NULL, init_game_for_thread, (void*)i) == -1) {
            log_fatal(monitor->log_file, "Failed to create thread #%d", i);
        }
    }
}

void handle_communication() {
}

int main(int argc, char *argv[]) {
    time_t t;
    srand((unsigned)time(&t));

    if (argc == 1)
    { // User does not input server configuration file
        log_fatal(NULL, "monitor <config>");
        return -1;
    }

    char *file_buffer;
    if (io_file_read(argv[1], &file_buffer) == -1)
    {
        log_fatal(NULL, "Failed to read file %s", argv[1]);
        return -1;
    }

    // Initialize monitor
    if (initialize_monitor(&monitor) == -1)
    {
        log_fatal(NULL, "Failed to initialize monitor");
        return -1;
    }

    // Parses monitor configuration file, and saves the settings to the monitor
    parse_monitor_config(file_buffer, &monitor);
    free(file_buffer);
    log_info(monitor->log_file, "Parsed monitor config");
    log_info(monitor->log_file, "Initialized monitor (ID: %s)", monitor->config->name);

    // Connect to server
    if (connect(monitor->socket_fd, (struct sockaddr *)&(monitor->socket_address), sizeof(monitor->socket_address)) == -1)
    {
        log_fatal(monitor->log_file, "Cannot connect to server");
        return -1;
    }

    build_guesses_array(&guesses);
    shuffle_guesses_array(&guesses);
    spawn_players();
    handle_communication();
    return 0;
}