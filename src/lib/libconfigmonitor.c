#include <arpa/inet.h>
#include <netinet/in.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/socket.h>
#include <time.h>
#include <unistd.h>

#include "libcom.h"
#include "libconfigmonitor.h"
#include "libconfigserver.h"
#include "libio.h"
#include "liblog.h"
#include "libutil.h"


// Closes the monitor log file, and frees the memory taken by the monitor
int clean_monitor(struct monitor_t *monitor)
{
    log_info(monitor->log_file, "Shutting down");
    close(monitor->socket_fd);
    free(monitor->config);
    free(monitor->threads);
    if (fclose(monitor->log_file) != 0)
    {
        return -1;
    }
    free(monitor);
    return 0;
}

// Creates the monitor's log file, initializes the monitor socket, and sets the configurations to the default values
int initialize_monitor(struct monitor_t **monitor)
{
    // Allocate memory for the monitor structure
    *monitor = (struct monitor_t*)malloc(sizeof(struct monitor_t));
    if ((*monitor) == NULL)
    { // Verify if memory was correctly allocated
        return -1;
    }
    // Allocate memory for the monitor configuration structure
    (*monitor)->config = calloc(1, sizeof(struct monitor_config_t));
    if ((*monitor)->config == NULL)
    { // Verify if memory was correctly allocated
        return -1;
    }

    // Creates log file and saves it to the monitor structure
    char *log_name = malloc(8 /* "MONITOR_" LENGTH */ + 10 /* TIMESTAMP LENGTH IN SEC. */ + 4 /* ".log" LENGTH */);
    sprintf(log_name, "MONITOR_%lu.log", (unsigned long)time(NULL)); // Sends formatted string to log_name
    (*monitor)->log_file = io_file_create(log_name);

    // Sets the monitor configuration values to the default ones
    (*monitor)->config->arrival_time_ms = DEFAULT_ARRIVAL_TIME;
    (*monitor)->config->threads = DEFAULT_THREADS;
    rand_string((*monitor)->config->name, 20);

    // Initialize monitor structure socket
    if (((*monitor)->socket_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        printf("Failed to create socket.\n");
        return -1;
    };
    bzero(&(*monitor)->socket_address, sizeof((*monitor)->socket_address));
    (*monitor)->socket_address.sin_family = AF_INET;
    (*monitor)->socket_address.sin_port = htons(DEFAULT_PORT);

    return 0;
}

// Parses the monitor configuration file, and saves it to the monitor structure
void parse_monitor_config(char *buffer, struct monitor_t **monitor)
{
    char *line = strtok(strdup(buffer), "\n");
    while (line)
    {
        if (sscanf(line, "arrival_time_ms = %u", &(*monitor)->config->arrival_time_ms) == 1)
        {
            line = strtok(NULL, "\n");
            continue;
        }

        char *server_address = calloc(1, INET_ADDRSTRLEN);
        if (sscanf(line, "server_address = %s", server_address) == 1)
        {
            if (inet_pton(AF_INET, server_address, &(*monitor)->socket_address.sin_addr) == -1)
            {
                log_error((*monitor)->log_file, "Invalid server address, using default localhost");
            }
            else
            {
                inet_pton(AF_INET, "127.0.0.1", &(*monitor)->socket_address.sin_addr);
            }

            line = strtok(NULL, "\n");
            continue;
        }

        if (sscanf(line, "threads = %u", &(*monitor)->config->threads) == 1)
        {
            line = strtok(NULL, "\n");
            continue;
        }

        else
        {
            printf("ERROR: invalid key `%s`\n", line);
            line = strtok(NULL, "\n");
        }
    }

    free(line);
}