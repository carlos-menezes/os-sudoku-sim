#include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "server.h"

#include "libarray.h"
#include "libcom.h"
#include "libconfigserver.h"
#include "libgrid.h"
#include "libio.h"
#include "liblog.h"
#include "libutil.h"

struct server_t *server;
pthread_mutex_t server_mutex;

struct linked_list_t *requests;
pthread_mutex_t requests_mutex;

struct game_state_t game_state;
pthread_mutex_t game_state_mutex;

pthread_t dispatch_thread, scheduler_thread, allow_init_thread;

volatile sig_atomic_t keep_running = 1;

/**
 * `init_game_state` selects a random grid for the simulation.
 */
void init_game_state(struct game_state_t *state)
{
    int rand_grid_index = rand_int(0, MAX_GRIDS - 1);
    game_state.grid.difficulty = server->grids[rand_grid_index].difficulty;
    strncpy(game_state.grid.problem, server->grids[rand_grid_index].problem, GRID_SIZE);
    strncpy(game_state.grid.solution, server->grids[rand_grid_index].solution, GRID_SIZE);
}

/**
 * For every new connection, a new thread runs `handle_monitor` to receive messages from a specific monitor.
 */
void *handle_monitor(void *socket_fd)
{
    while (keep_running)
    {
        struct monitor_msg_t *in_msg;
        in_msg = (struct monitor_msg_t *)malloc(sizeof(struct monitor_msg_t));
        if (recv((int *)socket_fd, in_msg, sizeof(struct monitor_msg_t), 0) <= 0) {
            break;
        }
        in_msg->socket_fd = (int*)socket_fd;
        pthread_mutex_lock(&requests_mutex);
        ll_insert(&requests, in_msg);
        log_info(server->log_file,
                 "RECV OK | NAME=%s TYPE=%d THREAD=%d REQUESTS=%d",
                 in_msg->monitor,
                 in_msg->type,
                 in_msg->thread_id,
                 requests->size);
        pthread_mutex_unlock(&requests_mutex);
    }
    close((int *)socket_fd);
    pthread_exit(NULL);
}

void handle_monitor_message(struct monitor_msg_t* msg) {
    switch (msg->type)
    {
    case MON_MSG_INIT:
        struct monitor_state_t* state;
        state = (struct monitor_state_t*)malloc(sizeof(struct monitor_state_t));
        strncpy(state->monitor, msg->monitor, MAX_MONITOR_NAME);
        state->priority = INITIAL_PRIORITY;
        state->socket_fd = msg->socket_fd;
        state->phase = STATE_WAITING_INIT;
        pthread_mutex_lock(&game_state_mutex);
        ll_insert(&(game_state.monitor_states), state);
        log_info(server->log_file, "HANDLED INIT MSG | MONITOR=%s", msg->monitor);
        pthread_mutex_unlock(&game_state_mutex);
        break;
    
    default:
        break;
    }
}

/**
 * `handle_communication()` is the communication loop which accepts new connections.
 */
void handle_communication()
{
    int new_socket_fd;
    struct sockaddr_in client_addr;
    socklen_t client_addr_len;
    client_addr_len = sizeof(client_addr);
    char client_ip[INET_ADDRSTRLEN];

    // Accept incoming connection
    while (keep_running &&
           (new_socket_fd = accept(server->socket_fd, (struct sockaddr *)&client_addr, &client_addr_len)))
    {
        inet_ntop(AF_INET, &(client_addr.sin_addr), client_ip, INET_ADDRSTRLEN);
        if (new_socket_fd == -1)
        {
            log_error(server->log_file, "ACCEPT ERROR | ERRNO=%d CLIENT=%s", errno, client_ip);
            continue;
        }
        log_info(server->log_file, "ACCEPT | CLIENT=%s", client_ip);

        pthread_mutex_lock(&game_state_mutex);
        struct node_t *cur = game_state.monitor_handlers->head;
        while (cur != NULL)
        {
            cur = cur->next;
        }

        cur = malloc(sizeof(struct node_t));
        cur->next = NULL;
        cur->value = malloc(sizeof(pthread_t));

        if (pthread_create(cur->value, NULL, handle_monitor, (void *)new_socket_fd) == -1)
        {
            log_error(server->log_file, "HANDLER THREAD CREATE ERROR | CLIENT=%s", client_ip);
        }
        else
        {
            log_info(server->log_file, "HANDLER THREAD CREATE OK | CLIENT=%s", client_ip);
        }
        pthread_mutex_unlock(&game_state_mutex);
    }
}

void* allow_init() {
    while (1) {
        pthread_mutex_lock(&game_state_mutex);
        if (game_state.monitor_states->size == server->config->min_monitors) {
            struct node_t* cur = game_state.monitor_states->head;
            while (cur != NULL) {
                // Send "START" message, allowing threads to play
                struct monitor_state_t* state = ((struct monitor_state_t*)(cur->value));
                struct server_msg_t out_msg;
                out_msg.type = SERV_MSG_START;
                if (send(state->socket_fd, &out_msg, sizeof(struct server_msg_t), 0) == -1) {
                    log_error(server->log_file, "SEND START FAIL | MONITOR=%s", state->monitor);
                    state->phase = STATE_DEAD; // TODO: restructure
                } else {
                    log_info(server->log_file, "SEND START OK | MONITOR=%s", state->monitor);
                    state->phase = STATE_GUESSING;
                    cur = cur->next;
                }
            }
            break;
        }
        pthread_mutex_unlock(&game_state_mutex);
        usleep(10000 * 2); // sleep for 200ms
    }
    pthread_exit(NULL);
}

void *dispatch()
{
    while (keep_running)
    {
        if (requests->size > 0)
        {
            pthread_mutex_lock(&requests_mutex);
            struct monitor_msg_t *msg = (struct monitor_msg_t *)(requests->head->value);
            handle_monitor_message(msg);
            ll_delete_value(&requests, requests->head->value);
            pthread_mutex_unlock(&requests_mutex);
        }
        usleep(DISPATCH_TRIGGER_TIME);
    }
    pthread_exit(NULL);
}

int cmpfunc(const void *a, const void *b)
{
    int a_priority, b_priority;
    struct node_t *cur = game_state.monitor_handlers->head;
    printf("%s\n", ((struct monitor_state_t *)(cur->value))->monitor);
    while (cur != NULL)
    {
        printf("%s\n", ((struct monitor_state_t *)(cur->value))->monitor);
        if (strcmp(((struct monitor_state_t *)(cur->value))->monitor, ((struct monitor_msg_t *)a)->monitor) == 0)
        {
            a_priority = ((struct monitor_state_t *)(cur->value))->priority;
        }
        else if (strcmp(((struct monitor_state_t *)(cur->value))->monitor, ((struct monitor_msg_t *)b)->monitor) == 0)
        {
            b_priority = ((struct monitor_state_t *)(cur->value))->priority;
        }
        printf("%s\n", ((struct monitor_state_t *)(cur->value))->monitor);
        cur = cur->next;
    }

    return a_priority - b_priority;
}

void *scheduler()
{
    while (keep_running)
    {
        pthread_mutex_lock(&requests_mutex);
        if (requests->size > 1)
        {
            struct node_t *cur = requests->head;
            while (cur != NULL)
            {
                printf("%s\n", ((struct monitor_msg_t *)(cur->value))->monitor);
                cur = cur->next;
            }
            printf("here2, size: %d\n", requests->size);
            qsort(requests, requests->size, sizeof(struct monitor_msg_t), cmpfunc);
            printf("here3, size: %d\n", requests->size);
            cur = requests->head;
            while (cur != NULL)
            {
                printf("%s\n", ((struct monitor_msg_t *)(cur->value))->monitor);
            }
        }
        pthread_mutex_unlock(&requests_mutex);
        usleep(SCHEDULER_TRIGGER_TIME);
    }
    pthread_exit(NULL);
}

void termination_handler(int _)
{
    keep_running = 0;
    pthread_cancel(scheduler_thread);
    pthread_cancel(dispatch_thread);

    ll_free(&requests);
    ll_free(&(game_state.monitor_states));
    ll_free(&(game_state.monitor_handlers));

    pthread_mutex_destroy(&server_mutex);
    pthread_mutex_destroy(&requests_mutex);
    for (size_t i = 0; i < GRID_SIZE; i++)
    {
        pthread_mutex_destroy(&(game_state.cell_mutex[i]));
    }

    clean_server(server);
    exit(EXIT_SUCCESS);
}

int main(int argc, char *argv[])
{
    time_t t;
    srand((unsigned)time(&t));

    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = termination_handler;
    sigemptyset(&sa.sa_mask);
    sigaction(SIGINT, &sa, NULL);

    // Initialize mutexes
    pthread_mutex_init(&server_mutex, NULL);
    pthread_mutex_init(&requests_mutex, NULL);
    for (size_t i = 0; i < GRID_SIZE; i++)
    {
        pthread_mutex_init(&(game_state.cell_mutex[i]), NULL);
    }

    // Initialize linked list
    ll_init(&requests);
    ll_init(&(game_state.monitor_states));
    ll_init(&(game_state.monitor_handlers));

    if (argc == 1)
    { // User inputs server configuration file
        log_fatal(NULL, "server <config>");
        exit(EXIT_FAILURE);
    }

    // Read user configuration file to buffer
    char *file_buffer;
    if (io_file_read(argv[1], &file_buffer) == -1)
    {
        log_fatal(NULL, "Failed to read config file `%s`", argv[1]);
        exit(EXIT_FAILURE);
    }

    // Initialize server
    if (initialize_server(&server) == -1)
    {
        log_fatal(NULL, "Failed to initialize server");
        exit(EXIT_FAILURE);
    }
    log_info(server->log_file, "Initialized server");

    // Parse server configuration file and free the associated buffer
    parse_server_config(file_buffer, &server);
    log_info(server->log_file, "Parsed server config");
    free(file_buffer);

    // Load Sudoku grids to server
    if (load_grids(&server) == -1)
    {
        log_fatal(server->log_file, "Failed to load `%s` file", GRIDS_FILE);
        exit(EXIT_FAILURE);
    };
    log_info(server->log_file, "Loaded grids");

    init_game_state(&game_state);
    log_info(server->log_file, "Initialized game state with grid #%u", game_state.grid.difficulty);

    if (listen(server->socket_fd, server->config->socket_backlog) == -1)
    {
        log_fatal(server->log_file, "Could not start listening to connections");
        exit(EXIT_FAILURE);
    }
    log_info(server->log_file, "Listening to incoming connections");

    log_info(server->log_file, "All initializations succeeded");

    // Initialize scheduler and dispatch threads
    pthread_create(&scheduler_thread, NULL, scheduler, NULL);
    pthread_create(&dispatch_thread, NULL, dispatch, NULL);
    pthread_create(&allow_init_thread, NULL, allow_init, NULL);

    handle_communication();

    pthread_join(scheduler_thread, NULL);
    pthread_join(dispatch_thread, NULL);
    pthread_join(allow_init_thread, NULL);
    return 0;
}