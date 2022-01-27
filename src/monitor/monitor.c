#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <stdio.h>
#include <semaphore.h>

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
sem_t can_handle_communication;

void *init_game_for_thread(void *thread_id)
{
    log_info(monitor->log_file, "INIT GAME | THREAD=%d", thread_id);
    while (keep_running) {
        struct monitor_msg_t out_msg;
        strncpy(out_msg.monitor, monitor->config->name, MAX_MONITOR_NAME);
        out_msg.thread_id = (unsigned int *)thread_id;
        out_msg.type = MON_MSG_GUESS;
        out_msg.guess = rand_int(1, 9);

        // Get available cells
        struct node_t* available_cells = NULL;
        pthread_mutex_lock(&monitor_mutex);
        for (size_t i = 0; i < GRID_SIZE; i++)
        {
            // Parse current cell value as an int
            int cellval_as_int = monitor->state.problem[i] - '0';
            // The current cell is free (i.e. '0')
            if (cellval_as_int == 0) {
                 // Insert current index in the linked list
                ll_insert(&available_cells, i);
            }
        }

        // Select a random index from the `available_cells` linked list
        int rand_index = rand_int(0, ll_size(available_cells) - 1);
        // Loop until count == rand_index to get to the value @ rand_index
        int count = 0;
        struct node_t* current = available_cells;
        while (count != rand_index)
        {
            count++;
            current = current->next;
        }
        
        out_msg.cell = (int)(current->value);
        pthread_mutex_unlock(&monitor_mutex);
        int delay = rand_int(1000, 3000);        

        if (send(monitor->socket_fd, &out_msg, sizeof(struct monitor_msg_t), 0) == -1) {
            log_error(monitor->log_file, "SEND ERROR, EXIT PROCESS | THREAD=%u", out_msg.thread_id);
        } else {
            log_info(monitor->log_file, "SEND OK | THREAD=%u TYPE=MON_MSG_GUESS CELL=%u GUESS=%u DELAY=%d", out_msg.thread_id, out_msg.cell, out_msg.guess, delay);
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

void handle_server_message(struct server_msg_t *in_msg) {
    switch (in_msg->type)
    {
    case SERV_MSG_OK: // Guess was correct
        log_info(monitor->log_file, "HANDLING SERVER OK | THREAD=%u", in_msg->thread_id);
        // Must mutate state with new problem
        pthread_mutex_lock(&monitor_mutex);
        strncpy(monitor->state.problem, in_msg->problem, GRID_SIZE);
        pthread_mutex_unlock(&monitor_mutex);
        log_info(monitor->log_file, "HANDLED SERVER OK | THREAD=%u", in_msg->thread_id);
        break;
    
    case SERV_MSG_ERR:
        log_info(monitor->log_file, "HANDLING SERVER ERR | THREAD=%u", in_msg->thread_id);
        log_info(monitor->log_file, "HANDLED SERVER ERR | THREAD=%u", in_msg->thread_id);
        break;
        
    case SERV_MSG_END:
        log_info(monitor->log_file, "HANDLING SERVER END");
        cleanup();
        log_info(monitor->log_file, "HANDLED SERVER END");
        break;
    default:
        break;
    }
}

void handle_communication()
{
    sem_wait(&can_handle_communication);
    while (keep_running)
    {
        struct server_msg_t in_msg;
        if (recv(monitor->socket_fd, &in_msg, sizeof(struct server_msg_t), 0) == -1) {
            log_error(monitor->log_file, "RECV FAIL");
        } else {
            handle_server_message(&in_msg);
        }
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
        pthread_mutex_lock(&monitor_mutex);
        strncpy(monitor->state.problem, in_msg.problem, GRID_SIZE);
        log_info(monitor->log_file, "RECV INIT OK | PROBLEM: %s", monitor->state.problem);
        pthread_mutex_unlock(&monitor_mutex);
    }
    return 0;
}

void cleanup() {
    keep_running = 0;    
    clean_monitor(monitor);
    exit(EXIT_SUCCESS);
}

void termination_handler(int _)
{
    cleanup();
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
    sem_init(&can_handle_communication, 0, 0);

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
    sem_post(&can_handle_communication);

    spawn_threads();
    handle_communication();
    clean_monitor(monitor);
    return 0;
}
