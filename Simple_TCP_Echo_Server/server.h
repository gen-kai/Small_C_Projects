#pragma once
#include <winsock2.h>


#define LISTENING_SOCKET_FAMILY     AF_INET
#define LISTENING_SOCKET_TYPE       SOCK_STREAM
#define LISTENING_SOCKET_PROTOCOL   IPPROTO_TCP
#define LISTENING_SOCKET_ADDRESS    INADDR_ANY
#define LISTENING_SOCKET_PORT       62'390U
#define SOCKET_POLL_TIMEOUT         10U
#define SOCKET_MAXIMUM_CONNECTIONS  1
#define REMOTE_IP_STRING_MAX_SIZE   15
// xxx.xxx.xxx.xxx - is 15 symbols
#define REMOTE_PORT_STRING_MAX_SIZE 5
#define REMOTE_ADDRESS_STRING_MAX_SIZE                            \
    (REMOTE_IP_STRING_MAX_SIZE + 1 + REMOTE_PORT_STRING_MAX_SIZE)
// IP address size + : + port size
#define MESSAGE_MAXIMUM_LENGTH 100U


int ConnectionLoop(SOCKET connectionSocket);
int SocketDispose(SOCKET socketToDispose);
