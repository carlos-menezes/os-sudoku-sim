#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <stdio.h>

#include "monitor.h"

#include "libarray.h"
#include "libconfigmonitor.h"
#include "libio.h"
#include "liblog.h"
#include "libcom.h"
#include "libutil.h"

struct monitor_t *monitor;
pthread_mutex_t monitor_mutex;
volatile sig_atomic_t keep_running = 1;

void *init_game_for_thread(void *thread_id)
{
    log_info(monitor->log_file, "INIT GAME | THREAD=%d", thread_id);
    while (keep_running) {
        struct monitor_msg_t out_msg;
        strncpy(out_msg.monitor, monitor->config->name, MAX_MONITOR_NAME);
        out_msg.thread_id = (unsigned int *)thread_id;
        out_msg.type = MON_MSG_GUESS;
        out_msg.cell = rand_int(0, 80);
        out_msg.guess = rand_int(1, 9);
        int delay = rand_int(1000, 3000);

        if (send(monitor->socket_fd, &out_msg, sizeof(struct monitor_msg_t), 0) == -1) {
            log_error(monitor->log_file, "SEND ERROR, EXIT PROCESS | THREAD=%u", out_msg.thread_id);
        } else {
            log_info(monitor->log_file, "SEND OK | THREAD=%u TYPE=MON_MSG_GUESS GUESS=%u@%u DELAY=%d", out_msg.thread_id, out_msg.guess, out_msg.cell, delay);
        }

        usleep((monitor->config->arrival_time_ms + delay) * 1000);
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
    while (keep_running)
    {
        /* code */
    }
    
}

/**
 * `handle_handshake()` sends the initial message with the intent to start playing and waits for a response from the server, indicating that the threads can be spawned and, thus, guesses can be sent via the socket.
 */

int handle_handshake() {
    struct monitor_msg_t out_msg;
    strncpy(out_msg.monitor, monitor->config->name, MAX_MONITOR_NAME);
    out_msg.type = MON_MSG_INIT;
    if (send(monitor->socket_fd, &out_msg, sizeof(struct monitor_msg_t), 0) == -1) {
        log_error(monitor->log_file, "SEND INIT ERROR");
        return -1;
    }
    log_info(monitor->log_file, "SEND INIT OK, WAIT RECV");
    struct server_msg_t in_msg;
    if (recv(monitor->socket_fd, &in_msg, sizeof(struct server_msg_t), 0) == -1) {
        log_error(monitor->log_file, "RECV INIT ERROR");
        return -1;
    } else {
        log_info(monitor->log_file, "RECV INIT OK");
    }
    return 0;
}

void termination_handler(int _)
{
    keep_running = 0;

    for (size_t i = 0; i < monitor->config->threads; i++)
    {
        pthread_cancel(monitor->threads + i);
    }
    
    clean_monitor(monitor);
    exit(EXIT_SUCCESS);
}

int main(int argc, char *argv[])
{
    time_t t;
    srand((unsigned)time(&t));

    struct sigaction sa;
    sa.sa_handler = termination_handler;
    sigemptyset(&sa.sa_mask);
    sigaction(SIGINT, &sa, NULL);

    pthread_mutex_init(&monitor_mutex, NULL);

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

    if (handle_handshake() == -1) {
        log_error(monitor->log_file, "Failed to send init message");
        exit(EXIT_FAILURE);
    };

    spawn_threads();

    handle_communication();
    return 0;
}
