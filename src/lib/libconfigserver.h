#ifndef LIBCONFIGSERVER_H
#define LIBCONFIGSERVER_H

#include <netinet/in.h>
#include <pthread.h>
#include <stdio.h>

#include "libgrid.h"

// Server configurations
typedef struct ServerConfig
{
	unsigned int socket_backlog;
} server_config_t;

// Server log, configurations, grids, socket and socket address
typedef struct Server
{
	FILE *log_file;
	server_config_t *config;
	grid_t grids[MAX_GRIDS];

	int socket_fd;
	struct sockaddr_in socket_address;
} server_t;

int initialize_server(server_t **server);
void parse_server_config(char *buffer, server_t **server);
int clean_server(server_t *server);
int load_grids(server_t **server);

#endif