#ifndef LIBCONFIGMONITOR_H
#define LIBCONFIGMONITOR_H

#include <netinet/in.h>
#include <pthread.h>
#include <stdio.h>

#define MAX_MONITOR_NAME 20

struct monitor_config_t
{
    unsigned int arrival_time_ms;
    unsigned int threads;
    char name[MAX_MONITOR_NAME];
};

// Monitor log file, configurations, player threads, socket and socket_address
struct monitor_t
{
    FILE *log_file;
    struct monitor_config_t *config;
    pthread_t *threads;

    int socket_fd;
    struct sockaddr_in socket_address;
};

int initialize_monitor(struct monitor_t** monitor);
void parse_monitor_config(char *buffer, struct monitor_t** monitor);
int clean_monitor(struct monitor_t* monitor);

#endif