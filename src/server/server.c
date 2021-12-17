#include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdlib.h>
#include <time.h>

#include "server.h"

#include "libconfigserver.h"
#include "libgrid.h"
#include "libio.h"
#include "liblog.h"

server_t *server;
pthread_mutex_t mutex;

/**
 * `handle_communication()` is the communication loop which accepts new connections and queues them
 */
void handle_communication()
{
    int new_socket_fd;
    struct sockaddr_in client_addr;
    socklen_t client_addr_len;
    client_addr_len = sizeof(client_addr);
    char client_ip[INET_ADDRSTRLEN];

    // Accept incoming connection
    while ((new_socket_fd = accept(server->socket_fd, (struct sockaddr *)&client_addr, &client_addr_len)))
    {
        inet_ntop(AF_INET, &(client_addr.sin_addr), client_ip, INET_ADDRSTRLEN);
        if (new_socket_fd == -1)
        {
            log_error(server->log_file, "ACCEPT ERROR | ERRNO=%d CLIENT=%s", errno, client_ip);
            continue;
        }
        log_info(server->log_file, "ACCEPT | CLIENT=%s", client_ip);
    }
}

void termination_handler(int _)
{
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

    if (listen(server->socket_fd, server->config->socket_backlog) == -1)
    {
        log_fatal(server->log_file, "Could not start listening to connections");
        exit(EXIT_FAILURE);
    }
    log_info(server->log_file, "Listening to incoming connections");

    log_info(server->log_file, "All initializations succeeded");
    handle_communication();
    return 0;
}