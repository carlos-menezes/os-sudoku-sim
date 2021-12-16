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
typedef struct MonitorMsg
{
    MONITOR_MESSAGE_TYPE type;
    unsigned int guess;
    unsigned int thread_id;
    char monitor[MAX_MONITOR_NAME];
} monitor_msg_t;
#pragma pack(0)

typedef enum
{
    SERV_MSG_OK,
    SERV_MSG_ERR,
    SERV_MSG_END
} SERVER_MESSAGE_TYPE;

#pragma pack(1)
typedef struct ServerMsg
{
    SERVER_MESSAGE_TYPE type;
    unsigned int thread_id;
    unsigned int guess;
} server_msg_t;
#pragma pack(0)

#endif