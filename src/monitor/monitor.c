#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#include "monitor.h"

#include "libarray.h"
#include "libconfigmonitor.h"
#include "libio.h"
#include "liblog.h"

monitor_t *monitor;

void *init_game_for_thread(void *thread_id)
{
    int id = (int *)thread_id;
    log_info(monitor->log_file, "INIT GAME | THREAD=%d", id);
    while (1)
    {
        log_info(monitor->log_file, "SLEEP | THREAD=%d", id);
        usleep(1000000);
    }
    pthread_exit(NULL);
}

void spawn_threads()
{
    monitor->threads = malloc(sizeof(pthread_t) * monitor->config->threads);
    for (size_t i = 0; i < monitor->config->threads; i++)
    {
        if (pthread_create(monitor->threads + i, NULL, init_game_for_thread, (void *)i) == -1)
        {
            log_fatal(monitor->log_file, "Failed to create thread #%d", i);
        }
    }
}

void handle_communication()
{
}

int main(int argc, char *argv[])
{
    time_t t;
    srand((unsigned)time(&t));

    if (argc == 1)
    { // User does not input server configuration file
        log_fatal(NULL, "monitor <config>");
        exit(EXIT_FAILURE);
    }

    char *file_buffer;
    if (io_file_read(argv[1], &file_buffer) == -1)
    {
        log_fatal(NULL, "Failed to read file %s", argv[1]);
        exit(EXIT_FAILURE);
    }

    // Initialize monitor
    if (initialize_monitor(&monitor) == -1)
    {
        log_fatal(NULL, "Failed to initialize monitor");
        exit(EXIT_FAILURE);
    }

    // Parses monitor configuration file, and saves the settings to the monitor
    parse_monitor_config(file_buffer, &monitor);
    free(file_buffer);
    log_info(monitor->log_file, "Parsed monitor config");
    log_info(monitor->log_file, "Initialized monitor (ID: %s)", monitor->config->name);

    // Connect to server
    if (connect(monitor->socket_fd, (struct sockaddr *)&(monitor->socket_address), sizeof(monitor->socket_address)) ==
        -1)
    {
        log_fatal(monitor->log_file, "Cannot connect to server");
        exit(EXIT_FAILURE);
    }
    log_info(monitor->log_file, "Connected to server");

    spawn_threads();
    log_info(monitor->log_file, "All initializations succeeded");

    // handle_communication();
    return 0;
}
