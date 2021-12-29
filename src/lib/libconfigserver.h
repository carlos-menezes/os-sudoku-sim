#ifndef LIBCONFIGSERVER_H
#define LIBCONFIGSERVER_H

#include <netinet/in.h>
#include <pthread.h>
#include <stdio.h>

#include "libgrid.h"

#define DEFAULT_SOCKET_BACKLOG 20
#define DEFAULT_MIN_MONITORS 1
#define DEFAULT_DISPATCH_BATCH 5

// Server configurations
struct server_config_t
{
	int socket_backlog;
	int min_monitors;
    int dispatch_batch;
};

// Server log, configurations, grids, socket and socket address
struct server_t
{
	FILE *log_file;
	struct server_config_t *config;
	struct grid_t grids[MAX_GRIDS];

	int socket_fd;
	struct sockaddr_in socket_address;
};

int initialize_server(struct server_t **server);
void parse_server_config(char *buffer, struct server_t **server);
int clean_server(struct server_t *server);
int load_grids(struct server_t **server);

#endif