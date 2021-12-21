#ifndef LIBCONFIGSERVER_H
#define LIBCONFIGSERVER_H

#include <netinet/in.h>
#include <pthread.h>
#include <stdio.h>

#include "libgrid.h"

// Server configurations
struct server_config_t
{
	unsigned int socket_backlog;
	unsigned int min_monitors;
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