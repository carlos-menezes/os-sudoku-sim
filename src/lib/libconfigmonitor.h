#ifndef LIBCONFIGMONITOR_H
#define LIBCONFIGMONITOR_H

#include <stdio.h>
#include <pthread.h>
#include <netinet/in.h>

#define MAX_MONITOR_NAME 20

typedef struct MonitorConfig
{
	unsigned int arrival_time_ms;
	unsigned int players;
	char name[MAX_MONITOR_NAME];
} monitor_config_t;

// Monitor log file, configurations, player threads, socket and socket_address
typedef struct Monitor
{
	FILE *log_file;
	monitor_config_t *config;
	pthread_t *players;

	int socket_fd;
	struct sockaddr_in socket_address;
	unsigned int current_guess_index;
} monitor_t;

int initialize_monitor(monitor_t **monitor);
void parse_monitor_config(char *buffer, monitor_t **monitor);
int clean_monitor(monitor_t *monitor);

#endif