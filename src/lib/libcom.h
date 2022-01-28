#ifndef LIBCOM_H
#define LIBCOM_H

#include "libconfigmonitor.h"
#include "libgrid.h"

#define DEFAULT_PORT 9998

typedef enum
{
    MON_MSG_INIT,
    MON_MSG_GUESS
} MONITOR_MESSAGE_TYPE;

#pragma pack(1)
struct monitor_msg_t
{
    unsigned int type;
    unsigned int guess;
    unsigned int cell;
    unsigned int priority;
    char monitor[MAX_MONITOR_NAME];
    unsigned int thread_id;
    unsigned int socket_fd;
};
#pragma pack(0)

typedef enum
{
    SERV_MSG_IGN,
    SERV_MSG_OK,
    SERV_MSG_ERR,
    SERV_MSG_END
} SERVER_MESSAGE_TYPE;

#pragma pack(1)
struct server_msg_t
{
    unsigned int type;
    unsigned int thread_id;
    unsigned int guess; // TODO: remove?
    char problem[GRID_SIZE + 1];
};
#pragma pack(0)

#endif