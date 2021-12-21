#ifndef LIBCOM_H
#define LIBCOM_H

#include "libconfigmonitor.h"

#define DEFAULT_PORT 9998

typedef enum
{
    MON_MSG_INIT,
    MON_MSG_GUESS
} MONITOR_MESSAGE_TYPE;

#pragma pack(1)
struct monitor_msg_t
{
    MONITOR_MESSAGE_TYPE type;
    unsigned int guess;
    unsigned int thread_id;
    unsigned int socket_fd;
    char monitor[MAX_MONITOR_NAME];
};
#pragma pack(0)

typedef enum
{
    SERV_MSG_START,
    SERV_MSG_OK,
    SERV_MSG_ERR,
    SERV_MSG_END
} SERVER_MESSAGE_TYPE;

#pragma pack(1)
struct server_msg_t
{
    SERVER_MESSAGE_TYPE type;
    unsigned int thread_id;
    unsigned int guess;
};
#pragma pack(0)

#endif