#include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <semaphore.h>

#include "server.h"

#include "libarray.h"
#include "libcom.h"
#include "libconfigserver.h"
#include "libgrid.h"
#include "libio.h"
#include "liblog.h"
#include "libutil.h"

struct server_t *server;
struct game_state_t game_info;
pthread_mutex_t game_info_mutex;

struct node_t* init_requests = NULL;
pthread_mutex_t init_requests_mutex;
pthread_t init_dispatch_thread;

struct node_t* requests = NULL;
pthread_mutex_t requests_mutex;
pthread_t dispatch_thread;

sem_t can_init_dispatch;

volatile sig_atomic_t keep_running = 1;

/**
 * `init_game_state` selects a random grid for the simulation.
 */
void init_game_state(struct game_state_t *state)
{
    int rand_grid_index = rand_int(0, MAX_GRIDS - 1);
    state->grid.difficulty = server->grids[rand_grid_index].difficulty;
    strncpy(state->grid.problem, server->grids[rand_grid_index].problem, GRID_SIZE);
    strncpy(state->grid.solution, server->grids[rand_grid_index].solution, GRID_SIZE);
    state->states = NULL;
    state->handlers = NULL;
    for (size_t i = 0; i < GRID_SIZE; i++)
    {
        pthread_mutex_init(&state->cell_mutex[i], NULL);
    }
}

void handle_monitor_message(struct monitor_msg_t* msg) {
    struct server_msg_t out_msg;
    switch (msg->type)
    {
    case MON_MSG_INIT:
        log_info(server->log_file, "HANDLING MESSAGE | MONITOR=%s TYPE=MON_MSG_INIT", msg->monitor);
        struct monitor_state_t *state;
        state = (struct monitor_state_t *)malloc(sizeof(struct monitor_state_t));
        strncpy(state->monitor, msg->monitor, MAX_MONITOR_NAME);
        state->priority = BASE_PRIORITY;
        state->socket_fd = msg->socket_fd;
        pthread_mutex_lock(&game_info_mutex);
        ll_insert(&(game_info.states), state);
        strncpy(out_msg.problem, game_info.grid.problem, GRID_SIZE);
        pthread_mutex_unlock(&game_info_mutex);
        out_msg.type = SERV_MSG_OK;
        if (send(msg->socket_fd, &out_msg, sizeof(struct server_msg_t), 0) <= 0) {
            log_info(server->log_file, "HANDLED MESSAGE, SEND OK | MONITOR=%s TYPE=MON_MSG_INIT", msg->monitor);
        } else {
            log_info(server->log_file, "HANDLED MESSAGE, SEND FAIL | MONITOR=%s TYPE=MON_MSG_INIT", msg->monitor);
        }
        break;
    case MON_MSG_GUESS:
        log_info(server->log_file, "HANDLING MESSAGE | MONITOR=%s TYPE=MON_MSG_GUESS GUESS=%u CELL=%u", msg->monitor, msg->guess, msg->cell);
        pthread_mutex_lock(&game_info_mutex);
        int solution = game_info.grid.solution[msg->cell] - '0';
        // Create outgoing message
        if (solution == msg->guess) { // Guess is correct
            // Modify the problem string with the guess
            game_info.grid.problem[msg->cell] = msg->guess + '0';
            
            // Check if there are any empty cells
            int empty_cells = 0;
            for (size_t i = 0; i < GRID_SIZE; i++)
            {
                int cellval_as_int = game_info.grid.problem[i] - '0';
                if (cellval_as_int == 0) { 
                    empty_cells = 1;
                    break;
                }
            }

            // Build the outgoing packet            
            if (empty_cells) { // Simulation must continue
                out_msg.type = SERV_MSG_OK;
                // Copy the problem into the `msg->problem` buffer
                strncpy(out_msg.problem, game_info.grid.problem, GRID_SIZE);
                out_msg.thread_id = msg->thread_id;
                pthread_mutex_unlock(&game_info_mutex);
            } else { // If there are no empty cells, the sudoku grid has been solved
                out_msg.type = SERV_MSG_END;
                log_info(server->log_file, "ENDING GAME");
                // Send the SERV_MSG_END message to every monitor
                struct node_t* current = game_info.states;
                while (current != NULL)
                {
                    struct monitor_state_t* state = current->value;
                    if (send(state->socket_fd, &out_msg, sizeof(struct server_msg_t), 0) > 0) {
                        log_info(server->log_file, "SEND END MESSAGE OK | MONITOR=%s TYPE=%u", state->monitor, out_msg.type);
                    }
                    current = current->next;
                }
                pthread_mutex_unlock(&game_info_mutex);
                cleanup();
                printf("here\n");
            }
        } else {
            pthread_mutex_unlock(&game_info_mutex);
            out_msg.type = SERV_MSG_ERR;
            out_msg.thread_id = msg->thread_id;
            if (send(msg->socket_fd, &out_msg, sizeof(struct server_msg_t), 0) <= 0) {
                log_info(server->log_file, "GUESS ERROR | SEND FAIL");
            } else {
                log_info(server->log_file, "GUESS ERROR | SEND OK");
            }
        }
        log_info(server->log_file, "HANDLED MESSAGE | MONITOR=%s TYPE=MON_MSG_GUESS GUESS=%u", msg->monitor, msg->guess);
        break;
    default:
        break;
    }
}

/**
 * For every new connection, a new thread runs `handle_monitor` to receive messages from a specific monitor.
 */
void *handle_monitor(void *socket_fd)
{
    while (keep_running)
    {
        struct monitor_msg_t *in_message;
        in_message = (struct monitor_msg_t *)malloc(sizeof(struct monitor_msg_t));
        if (recv((int *)socket_fd, in_message, sizeof(struct monitor_msg_t), 0) <= 0) {
            break;
        }
        in_message->socket_fd = (unsigned int*)socket_fd;
        switch (in_message->type)
        {
        case MON_MSG_INIT:
            pthread_mutex_lock(&init_requests_mutex);
            ll_insert(&init_requests, in_message);
            pthread_mutex_unlock(&init_requests_mutex);
            log_info(server->log_file, "RECV OK | NAME=%s TYPE=MON_MSG_INIT", in_message->monitor);
            break;
        case MON_MSG_GUESS:
            pthread_mutex_lock(&requests_mutex);
            ll_insert(&requests, in_message);
            pthread_mutex_unlock(&requests_mutex);
            log_info(server->log_file, "RECV OK | NAME=%s TYPE=MON_MSG_GUESS THREAD=%u", in_message->monitor, in_message->thread_id);
            break;
        }
    }
    close((int *)socket_fd);
    pthread_exit(NULL);
}

void* init_dispatch() {
    log_info(server->log_file, "INIT DISPATCH START");
    int count = 0;
    while (1) {
        while (count < server->config->dispatch_batch) {
            pthread_mutex_lock(&init_requests_mutex);
            struct node_t* current = init_requests;
            if (current != NULL) {
                count += 1;
                struct monitor_msg_t* msg = (struct monitor_msg_t*)(current->value);
                log_info(server->log_file, "HANDLING MESSAGE | MONITOR=%s TYPE=MON_MSG_INIT", msg->monitor);
                // THIS SHOULD HAVE BEEN HANDLED IN handle_monitor_msg :/
                struct monitor_state_t *state;
                state = (struct monitor_state_t *)malloc(sizeof(struct monitor_state_t));
                strncpy(state->monitor, msg->monitor, MAX_MONITOR_NAME);
                state->priority = BASE_PRIORITY;
                state->socket_fd = msg->socket_fd;
                pthread_mutex_lock(&game_info_mutex);
                ll_insert(&(game_info.states), state);
                pthread_mutex_unlock(&game_info_mutex);
                log_info(server->log_file, "HANDLED MESSAGE | MONITOR=%s TYPE=MON_MSG_INIT", msg->monitor);
                // IT SHOULD REALLY HAVE BEEN
                ll_delete_value(&init_requests, current->value);
                pthread_mutex_unlock(&init_requests_mutex);
            } else {
                pthread_mutex_unlock(&init_requests_mutex);
                break;
            }
        }

        // Enough monitors joined, game can start
        pthread_mutex_lock(&game_info_mutex);
        if (ll_size(game_info.states) == server->config->min_monitors) {
            struct node_t* current = game_info.states;
            while (current != NULL) {
                // Send START message
                struct monitor_state_t* state = ((struct monitor_state_t*)(current->value));
                struct server_msg_t out_msg;
                out_msg.type = SERV_MSG_OK;
                strncpy(out_msg.problem, game_info.grid.problem, GRID_SIZE);
                if (send(state->socket_fd, &out_msg, sizeof(struct server_msg_t), 0) <= 0) {
                    log_error(server->log_file, "SEND START FAIL | MONITOR=%s", state->monitor);
                    ll_delete_value(&(game_info.states), current->value);
                } else {
                    log_info(server->log_file, "SEND START OK | MONITOR=%s", state->monitor);
                    current = current->next;
                }
            }
            pthread_mutex_unlock(&game_info_mutex);
            break;
        }
        pthread_mutex_unlock(&game_info_mutex);
        count = 0;
        usleep(DISPATCH_TRIGGER_TIME);
    }
    log_info(server->log_file, "INIT DISPATCH END");
    sem_post(&can_init_dispatch);
    pthread_exit(NULL);
}

void *dispatch()
{
    sem_wait(&can_init_dispatch);
    log_info(server->log_file, "INIT DISPATCH THREAD");
    int count = 0;
    while (keep_running)
    {
        pthread_mutex_lock(&init_requests_mutex);
        pthread_mutex_lock(&requests_mutex);
        log_info(server->log_file, "DISPATCH START | BATCH=%d INIT_REQUESTS=%d REQUESTS=%d", server->config->dispatch_batch, ll_size(init_requests), ll_size(requests));
        pthread_mutex_unlock(&init_requests_mutex);
        pthread_mutex_unlock(&requests_mutex);
        while (count < server->config->dispatch_batch) {
            count += 1;
            pthread_mutex_lock(&init_requests_mutex);
            // init_requests works in FIFO
            if (ll_size(init_requests) > 0)
            {
                struct monitor_msg_t *msg = (struct monitor_msg_t *)(init_requests->value);
                handle_monitor_message(msg);
                ll_delete_value(&init_requests, init_requests->value);
                pthread_mutex_unlock(&init_requests_mutex);
                continue;
            }
            pthread_mutex_unlock(&init_requests_mutex);

            // requests works in fifo (eventually priority)
            pthread_mutex_lock(&requests_mutex);
            if (ll_size(requests) > 0)
            {
                struct monitor_msg_t *msg = (struct monitor_msg_t *)(requests->value);
                handle_monitor_message(msg);
                ll_delete_value(&requests, requests->value);
                pthread_mutex_unlock(&requests_mutex);
                continue;
            }
            pthread_mutex_unlock(&requests_mutex);
        }
        count = 0;
        log_info(server->log_file, "DISPATCH END");
        usleep(DISPATCH_TRIGGER_TIME);
    }
    log_info(server->log_file, "END DISPATCH THREAD");
    pthread_exit(NULL);
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

        pthread_t* handler_thread;
        handler_thread = (pthread_t*)malloc(sizeof(pthread_t));
        if (pthread_create(handler_thread, NULL, handle_monitor, (void *)new_socket_fd) != 0)
        {
            log_error(server->log_file, "HANDLER THREAD CREATE ERROR | CLIENT=%s", client_ip);
            free(handler_thread);
        }
        else
        {
            log_info(server->log_file, "HANDLER THREAD CREATE OK | CLIENT=%s", client_ip);
            pthread_mutex_lock(&game_info_mutex);
            ll_insert(&(game_info.handlers), handler_thread);
            pthread_mutex_unlock(&game_info_mutex);
        }
    }
}

void cleanup() {
    keep_running = 0;
    pthread_cancel(init_dispatch_thread);
    pthread_cancel(dispatch_thread);

    ll_free(&(requests));
    ll_free(&(init_requests));
    ll_free(&(game_info.states));
    ll_free(&(game_info.handlers));

    pthread_mutex_destroy(&game_info_mutex);
    for (size_t i = 0; i < GRID_SIZE; i++)
    {
        pthread_mutex_destroy(&(game_info.cell_mutex[i]));
    }

    clean_server(server);
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
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = termination_handler;
    sigemptyset(&sa.sa_mask);
    sigaction(SIGINT, &sa, NULL);

    // Initialize semaphore
    sem_init(&can_init_dispatch, 0, 0);

    // Initialize mutexes
    pthread_mutex_init(&init_requests_mutex, NULL);
    pthread_mutex_init(&requests_mutex, NULL);

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

    init_game_state(&game_info);
    log_info(server->log_file, "Initialized game state with grid #%u", game_info.grid.difficulty);

    if (listen(server->socket_fd, server->config->socket_backlog) == -1)
    {
        log_fatal(server->log_file, "Could not start listening to connections");
        exit(EXIT_FAILURE);
    }
    log_info(server->log_file, "Listening to incoming connections");

    log_info(server->log_file, "All initializations succeeded");

    // Initialize dispatch threads
    pthread_create(&init_dispatch_thread, NULL, init_dispatch, NULL);
    pthread_create(&dispatch_thread, NULL, dispatch, NULL);

    handle_communication();

    pthread_join(dispatch_thread, NULL);
    pthread_join(init_dispatch_thread, NULL);
    return 0;
}