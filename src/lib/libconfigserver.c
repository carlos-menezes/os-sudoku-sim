#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/socket.h>
#include <time.h>
#include <unistd.h>

#include "libcom.h"
#include "libconfigserver.h"
#include "libio.h"
#include "liblog.h"

// Closes the server log file, and frees the memory taken by the server
int clean_server(struct server_t *server)
{
    log_info(server->log_file, "SHUTTING DOWN SOCKET");
    close(server->socket_fd);
    if (fclose(server->log_file) != 0)
    {
        return -1;
    }
    return 0;
}

// Creates the server's log file, and sets the configurations to the default values
int initialize_server(struct server_t **server)
{
    *server = (struct server_t *)malloc(sizeof(struct server_t));
    if ((*server) == NULL)
    {
        return -1;
    }

    (*server)->config = calloc(1, sizeof(struct server_config_t));
    if ((*server)->config == NULL)
    {
        return -1;
    }

    (*server)->config->dispatch_batch = DEFAULT_DISPATCH_BATCH;
    (*server)->config->min_monitors = DEFAULT_MIN_MONITORS;
    (*server)->config->socket_backlog = DEFAULT_SOCKET_BACKLOG;

    // Create log file
    char *log_name = malloc(8 /* "MONITOR_" LENGTH */ + 10 /* TIMESTAMP LENGTH IN SEC. */ + 4 /* ".log" LENGTH */);
    sprintf(log_name, "SERVER_%lu.log", (unsigned long)time(NULL));
    (*server)->log_file = io_file_create(log_name);
    free(log_name);

    // Initialize socket
    if (((*server)->socket_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        log_fatal(NULL, "Failed to create socket");
        return -1;
    };

    // Initialize socket structure
    bzero(&(*server)->socket_address, sizeof((*server)->socket_address));
    (*server)->socket_address.sin_family = AF_INET;
    (*server)->socket_address.sin_addr.s_addr = INADDR_ANY;
    (*server)->socket_address.sin_port = htons(DEFAULT_PORT);

    setsockopt((*server)->socket_fd, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int));
    if ((bind((*server)->socket_fd,
              (struct sockaddr *)&(*server)->socket_address,
              sizeof((*server)->socket_address))) == -1)
    {
        log_fatal(NULL, "Failed to bind");
        return -1;
    }

    log_info((*server)->log_file,
             "Socket created on %d:%d",
             (*server)->socket_address.sin_addr.s_addr,
             ntohs((*server)->socket_address.sin_port));
    return 0;
};

// Reads the configuration file, and sets the configurations to those new values
void parse_server_config(char *buffer, struct server_t **server)
{
    char *line = strtok(strdup(buffer), "\n");
    while (line)
    {
        if (sscanf(line, "socket_backlog = %d", &(*server)->config->socket_backlog) == 1)
        {
            line = strtok(NULL, "\n");
            continue;
        }

        if (sscanf(line, "min_monitors = %d", &(*server)->config->min_monitors) == 1)
        {
            line = strtok(NULL, "\n");
            continue;
        }

        if (sscanf(line, "dispatch_batch = %d", &(*server)->config->dispatch_batch))
        {
            line = strtok(NULL, "\n");
            continue;
        }

        else
        {
            log_error((*server)->log_file, "INVALID KEY | KEY=`%s`", line);
            line = strtok(NULL, "\n");
        }
    }

    free(line);
};
