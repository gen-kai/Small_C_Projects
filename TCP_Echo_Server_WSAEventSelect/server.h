#pragma once
#include <winsock2.h>
#include <windows.h>


#pragma comment(lib, "ws2_32.lib")


#define LISTENING_SOCKET_FAMILY   AF_INET
#define LISTENING_SOCKET_TYPE     SOCK_STREAM
#define LISTENING_SOCKET_PROTOCOL IPPROTO_TCP
#define LISTENING_SOCKET_ADDRESS  INADDR_ANY
#define LISTENING_SOCKET_PORT     5000U

#define DEFAULT_BUFFER_SIZE 4096

#define REMOTE_IP_STRING_MAX_SIZE   15
// xxx.xxx.xxx.xxx - is 15 symbols
#define REMOTE_PORT_STRING_MAX_SIZE 5
#define REMOTE_ADDRESS_STRING_MAX_SIZE                              \
    (                                                               \
        REMOTE_IP_STRING_MAX_SIZE + 1 + REMOTE_PORT_STRING_MAX_SIZE \
    ) // IP address size + : symbol + port size


typedef struct
{
    SOCKET socketDescriptor;
    char   dataBuffer[DEFAULT_BUFFER_SIZE];
    UINT   dataBufferBytesOccupied;
    WSABUF wsaBufStructure;
} SESSION_INFO;


int CreateListeningSocket(
    LPSOCKADDR_IN p_listeningSocketAddress,
    int           listeningSocketAddressLength
);

int  CreateSession(SOCKET socketDescriptor);
void DestroySession(int socketIndex);

int  SocketAccept(int socketIndex);
int  SocketRead(int eventIndex);
int  SocketWrite(int eventIndex);
void SocketClose(int socketIndex);

void SocketResolveAddress(
    LPSOCKADDR p_remoteAddress,
    DWORD      remoteAddressLength
);

void Cleanup();
