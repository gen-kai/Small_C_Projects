#include <stdio.h>
#include <winsock2.h>
#include "server.h"


int           socketCount = 0;
WSAEVENT      eventList[WSA_MAXIMUM_WAIT_EVENTS];
SESSION_INFO* sessionList[WSA_MAXIMUM_WAIT_EVENTS];


int main(int argCount, char* argValues[])
{
    WORD    wsaVersion = MAKEWORD(2, 2);
    WSADATA wsaData;


    SOCKADDR_IN listeningSocketAddress = {
        .sin_family      = LISTENING_SOCKET_FAMILY,
        .sin_port        = htons(LISTENING_SOCKET_PORT),
        .sin_addr.s_addr = LISTENING_SOCKET_ADDRESS
    };

    WSANETWORKEVENTS networkEvents;


    int wsaStartupResult = WSAStartup(wsaVersion, &wsaData);

    if (wsaStartupResult != 0)
    {
        printf("WSA Startup failed with error %d\n", wsaStartupResult);


        return -1;
    }

    if (LOBYTE(wsaData.wVersion) != 2 || HIBYTE(wsaData.wVersion) != 2)
    {
        printf("Could not find a usable version of Winsock.dll\n");


        WSACleanup();
        return -1;
    }
    else
    {
        printf(
            "Winsock.dll of version %d.%d was loaded\n",
            LOBYTE(wsaData.wVersion),
            HIBYTE(wsaData.wVersion)
        );
    }


    if (CreateListeningSocket(
            &listeningSocketAddress,
            sizeof(listeningSocketAddress)
        )
        != 0)
    {
        printf("Listening socket:\n");


        Cleanup();


        return -1;
    }


    while (TRUE)
    {
        int eventIndex = WSAWaitForMultipleEvents(
            socketCount,
            eventList,
            FALSE,
            WSA_INFINITE,
            FALSE
        );

        if (eventIndex == WSA_WAIT_FAILED)
        {
            printf(
                "WSAWaitForMultipleEvents() failed with error %d\n",
                WSAGetLastError()
            );


            for (int socketIndex = 0; socketIndex < socketCount; socketIndex++)
            {
                printf("Socket %d:\n", socketIndex);
                SocketClose(socketIndex);
            }


            break;
        }
        else
        {
            eventIndex -= WSA_WAIT_EVENT_0;


            printf("Socket %d signalled back!\n", eventIndex);
        }


        int wsaEnumNetworkEventsResult = WSAEnumNetworkEvents(
            sessionList[eventIndex]->socketDescriptor,
            eventList[eventIndex],
            &networkEvents
        );

        if (wsaEnumNetworkEventsResult == SOCKET_ERROR)
        {
            printf(
                "\tWSAEnumNetworkEvents() failed with error %d\n",
                WSAGetLastError()
            );


            for (int socketIndex = 0; socketIndex < socketCount; socketIndex++)
            {
                printf("Socket %d:\n", socketIndex);
                SocketClose(socketIndex);
            }


            break;
        }
        else
        {
            printf("\tEnumerated network events for socket %d!\n", eventIndex);
        }


        if (networkEvents.lNetworkEvents & FD_ACCEPT)
        {
            if (networkEvents.iErrorCode[FD_ACCEPT_BIT] != 0)
            {
                printf(
                    "\tFD_ACCEPT failed with error %d\n",
                    networkEvents.iErrorCode[FD_ACCEPT_BIT]
                );


                break;
            }
            else
            {
                printf("\tFD_ACCEPT was successful!\n");
            }


            if (SocketAccept(eventIndex) != 0)
            {
                continue;
            }


            printf("Current number of sockets: %d\n", socketCount);
        }
        // We can write to the socket since the moment of its creation so after
        // FD_READ we call ReadSocket() -> WriteSocket()
        else if (networkEvents.lNetworkEvents & FD_READ)
        {
            if (networkEvents.iErrorCode[FD_READ_BIT] != 0)
            {
                printf(
                    "\tFD_READ failed with error %d\n",
                    networkEvents.iErrorCode[FD_READ_BIT]
                );


                // exit the loop and clean up
                break;
            }
            else
            {
                printf("\tFD_READ was successful!\n");
            }

            if (SocketRead(eventIndex) != 0)
            {
                // exit the loop and clean up
                break;
            }

            if (SocketWrite(eventIndex) != 0)
            {
                // exit the loop and clean up
                break;
            }
        }
        else if (networkEvents.lNetworkEvents & FD_WRITE)
        {
            if (networkEvents.iErrorCode[FD_WRITE_BIT] != 0)
            {
                printf(
                    "\tFD_WRITE failed with error %d\n",
                    networkEvents.iErrorCode[FD_WRITE_BIT]
                );


                // exit the loop and clean up
                break;
            }
            else
            {
                printf("\tFD_WRITE was successful!\n");
            }


            if (SocketWrite(eventIndex) != 0)
            {
                // exit the loop and clean up
                break;
            }
        }
        else if (networkEvents.lNetworkEvents & FD_CLOSE)
        {
            if (networkEvents.iErrorCode[FD_CLOSE_BIT] != 0)
            {
                printf(
                    "\tFD_CLOSE failed with error %d\n",
                    networkEvents.iErrorCode[FD_CLOSE_BIT]
                );


                // exit the loop and clean up
                break;
            }
            else
            {
                printf("\tFD_CLOSE was successful!\n");
            }


            SocketClose(eventIndex);


            printf("Current number of sockets: %d\n", socketCount);
        }
    };


    Cleanup();


    return 0;
}

int CreateListeningSocket(
    LPSOCKADDR_IN p_listeningSocketAddress,
    int           listeningSocketAddressLength
)
{
    SOCKET listeningSocket = INVALID_SOCKET;


    listeningSocket = socket(
        LISTENING_SOCKET_FAMILY,
        LISTENING_SOCKET_TYPE,
        LISTENING_SOCKET_PROTOCOL
    );

    if (listeningSocket == INVALID_SOCKET)
    {
        printf("Socket creation failed with error %d\n", WSAGetLastError());


        WSACleanup();
        return -1;
    }
    else
    {
        printf("Unnamed socket was created!\n");
    }


    int createSocketObjectResult = CreateSession(listeningSocket);

    if (createSocketObjectResult != 0)
    {
        printf("Unnamed socket:\n");


        int shutdownResult = shutdown(listeningSocket, SD_BOTH);

        if (shutdownResult != SOCKET_ERROR)
        {
            printf("\tSocket was shutdown!\n");
        }
        else
        {
            printf(
                "\tSocket shutdown error. Error code: %d!\n",
                WSAGetLastError()
            );
        }


        int closesocketResult = closesocket(listeningSocket);

        if (closesocketResult != SOCKET_ERROR)
        {
            printf("\tSocket was closed!\n");
        }
        else
        {
            printf(
                "\tSocket close error. Error code: %d!\n",
                WSAGetLastError()
            );
        }


        return -1;
    }
    else
    {
        printf("Unnamed socket information strcuture was created!\n");
    }


    int bindResult = bind(
        listeningSocket,
        (SOCKADDR*)p_listeningSocketAddress,
        listeningSocketAddressLength
    );

    if (bindResult == SOCKET_ERROR)
    {
        printf(
            "Socket bind was unsuccessfull. Error code: %d\n",
            WSAGetLastError()
        );


        return -1;
    }
    else
    {
        printf("Socket was bound!\n");
    }


    int listenResult = listen(listeningSocket, WSA_MAXIMUM_WAIT_EVENTS - 1);

    if (listenResult == SOCKET_ERROR)
    {
        printf(
            "Socket listen was unsuccessfull. Error code: %d\n",
            WSAGetLastError()
        );


        return -1;
    }
    else
    {
        printf("Socket was switched into listen state!\n");
    }


    int registerAcceptEvent = WSAEventSelect(
        listeningSocket,
        eventList[socketCount - 1],
        FD_ACCEPT | FD_CLOSE
    );

    if (registerAcceptEvent == SOCKET_ERROR)
    {
        printf(
            "Registering Accept event failed with error %d\n",
            WSAGetLastError()
        );


        return -1;
    }
    else
    {
        printf("Accept event was registered!\n");
    }


    return 0;
}

int CreateSession(SOCKET socketDescriptor)
{
    WSAEVENT newWSAEvent = WSACreateEvent();

    if (newWSAEvent == WSA_INVALID_EVENT)
    {
        printf(
            "\tEvent creation for socket failed with error %d\n",
            WSAGetLastError()
        );


        return -1;
    }

    eventList[socketCount] = newWSAEvent;


    SESSION_INFO* p_sessionInfo = HeapAlloc(
        GetProcessHeap(),
        HEAP_ZERO_MEMORY,
        sizeof(SESSION_INFO)
    );

    if (p_sessionInfo == NULL)
    {
        printf("\tCreation of the SESSION_INFO structure failed!\n");


        return -1;
    }


    p_sessionInfo->socketDescriptor        = socketDescriptor;
    p_sessionInfo->dataBufferBytesOccupied = 0;
    p_sessionInfo->wsaBufStructure.buf     = p_sessionInfo->dataBuffer;
    p_sessionInfo->wsaBufStructure.len     = DEFAULT_BUFFER_SIZE;


    sessionList[socketCount] = p_sessionInfo;
    socketCount++;
    return 0;
}

void DestroySession(int socketIndex)
{
    HeapFree(GetProcessHeap(), 0, sessionList[socketIndex]);


    if (WSACloseEvent(eventList[socketIndex]) == TRUE)
    {
        printf("\tWSACloseEvent() was successful!\n");
    }
    else
    {
        printf("\tWSACloseEvent() failed!\n");


        if (WSAGetLastError() == WSA_INVALID_HANDLE)
        {
            printf("\tNo event fount!\n");
        }
    }


    for (int elementIndex = socketIndex; elementIndex < socketCount;
         elementIndex++)
    {
        eventList[elementIndex] = eventList[elementIndex + 1];

        sessionList[elementIndex] = sessionList[elementIndex + 1];
    }
}

int SocketAccept(int eventIndex)
{
    SOCKADDR remoteAddress;
    int      remoteAddressLength = sizeof(remoteAddress);


    if (socketCount >= WSA_MAXIMUM_WAIT_EVENTS)
    {
        printf("\tToo many connections, continue with the next event!\n");

        return 0;
    }


    SOCKET connectionSocket = accept(
        sessionList[eventIndex]->socketDescriptor,
        &remoteAddress,
        &remoteAddressLength
    );

    if (connectionSocket == INVALID_SOCKET)
    {
        printf(
            "\tConnection accept on socket %d was unsuccessfull. "
            "Error code: %d\n",
            eventIndex,
            WSAGetLastError()
        );


        return -1;
    }
    else
    {
        printf("\tNew connection established!\n");


        SocketResolveAddress(&remoteAddress, remoteAddressLength);
    }


    int createSocketObjectResult = CreateSession(connectionSocket);

    if (createSocketObjectResult != 0)
    {
        printf("Connection socket:\n");


        int shutdownResult = shutdown(connectionSocket, SD_BOTH);

        if (shutdownResult != SOCKET_ERROR)
        {
            printf("\tSocket was shutdown!\n");
        }
        else
        {
            printf(
                "\tSocket shutdown error. Error code: %d!\n",
                WSAGetLastError()
            );
        }


        int closesocketResult = closesocket(connectionSocket);

        if (closesocketResult != SOCKET_ERROR)
        {
            printf("\tSocket was closed!\n");
        }
        else
        {
            printf(
                "\tSocket close error. Error code: %d!\n",
                WSAGetLastError()
            );
        }


        return -1;
    }

    printf("\tConnection socket information structure was created!\n");


    int registerConnectionSocket = WSAEventSelect(
        sessionList[socketCount - 1]->socketDescriptor,
        eventList[socketCount - 1],
        FD_READ | FD_CLOSE
    );

    if (registerConnectionSocket == SOCKET_ERROR)
    {
        printf(
            "\tRegistering connection socket failed with error %d\n",
            WSAGetLastError()
        );

        printf("Listening socket:\n");
        SocketClose(socketCount - 1);


        return -1;
    }
    else
    {
        printf("\tConnection socket was registered!\n");
    }


    return 0;
}

// Receive from socket, select FD_READ, send to socket
int SocketRead(int eventIndex)
{
    // READ from socket, set buffer bytes occupied, select FD_WRITE
    DWORD recvFlags = 0;
    if (sessionList[eventIndex]->dataBufferBytesOccupied == 0)
    {
        int wsaRecvResult = WSARecv(
            sessionList[eventIndex]->socketDescriptor,
            &(sessionList[eventIndex]->wsaBufStructure),
            1,
            &(sessionList[eventIndex]->dataBufferBytesOccupied),
            &recvFlags,
            NULL,
            NULL
        );

        if (wsaRecvResult == SOCKET_ERROR)
        {
            printf("\tWSARecv() failed with error %d\n", WSAGetLastError());


            return -1;
        }
        else
        {
            printf(
                "\tWSARecv() was successful. Received %d bytes!\n",
                sessionList[eventIndex]->dataBufferBytesOccupied
            );
        }

        // Set WSABUF length to the number of bytes recieved
        sessionList[eventIndex]->wsaBufStructure.len =
            sessionList[eventIndex]->dataBufferBytesOccupied;


        int registerWriteEvent = WSAEventSelect(
            sessionList[eventIndex]->socketDescriptor,
            eventList[eventIndex],
            FD_READ | FD_CLOSE
        );

        if (registerWriteEvent == SOCKET_ERROR)
        {
            printf(
                "\tRegistering FD_WRITE event after FD_READ failed with "
                "error %d\n",
                WSAGetLastError()
            );


            return -1;
        }
        else
        {
            printf("\tThe next event after FD_READ was registered!\n");
        }
    }


    return 0;
}

// Send buffer to socket; reset bytes occupied; select FD_READ if the entire
// buffer was sent, register new FD_WRITE event if not
int SocketWrite(int eventIndex)
{
    DWORD sendFlags               = 0;
    DWORD bytesSent               = 0;
    long  networkEventsToRegister = 0;

    if (sessionList[eventIndex]->dataBufferBytesOccupied != 0)
    {
        int wsaSendResult = WSASend(
            sessionList[eventIndex]->socketDescriptor,
            &(sessionList[eventIndex]->wsaBufStructure),
            1,
            &bytesSent,
            sendFlags,
            NULL,
            NULL
        );

        if ((wsaSendResult == SOCKET_ERROR)
            && (WSAGetLastError() != WSAEWOULDBLOCK))
        {
            printf("\tWSASend() failed with error %d\n", WSAGetLastError());
        }
        else if ((wsaSendResult == SOCKET_ERROR)
                 && (WSAGetLastError() == WSAEWOULDBLOCK))
        {
            printf(
                "\tWSASend() returned WSAEWOULDBLOCK. The send operation "
                "cannot be completed immediately! Rescheduling..."
            );
        }
        else
        {
            printf("\tWSASend() was successful. Sent %d bytes!\n", bytesSent);
        }


        sessionList[eventIndex]->dataBufferBytesOccupied -= bytesSent;


        // if we didn't send the whole buffer - register new write
        // event, otherwise - read and set WSABUF length to the number
        // of bytes left after WSASend
        if ((sessionList[eventIndex]->dataBufferBytesOccupied != 0)
            || (WSAGetLastError() == WSAEWOULDBLOCK))
        {
            networkEventsToRegister = FD_WRITE | FD_CLOSE;
            sessionList[eventIndex]->wsaBufStructure.len =
                sessionList[eventIndex]->dataBufferBytesOccupied;
        }
        else
        {
            networkEventsToRegister                      = FD_READ | FD_CLOSE;
            sessionList[eventIndex]->wsaBufStructure.len = DEFAULT_BUFFER_SIZE;
        }

        int registerNextEvent = WSAEventSelect(
            sessionList[eventIndex]->socketDescriptor,
            eventList[eventIndex],
            networkEventsToRegister
        );

        if (registerNextEvent == SOCKET_ERROR)
        {
            printf(
                "\tRegistering the next event after FD_WRITE failed with "
                "error %d\n",
                WSAGetLastError()
            );


            return -1;
        }
        else
        {
            if (networkEventsToRegister & FD_READ == FD_READ)
            {
                printf("\tFD_READ event after FD_WRITE was registered!\n");
            }
            else
            {
                printf("\tFD_WRITE event after FD_WRITE was registered!\n");
            }
        }
    }
    else
    {
        int registerNextEvent = WSAEventSelect(
            sessionList[eventIndex]->socketDescriptor,
            eventList[eventIndex],
            FD_READ | FD_CLOSE
        );

        if (registerNextEvent == SOCKET_ERROR)
        {
            printf(
                "\tRegistering FD_READ event after FD_WRITE failed with "
                "error %d\n",
                WSAGetLastError()
            );


            return -1;
        }
        else
        {
            printf("\tFD_READ event after FD_WRITE was registered!\n");
        }
    }


    return 0;
}

void SocketClose(int socketIndex)
{
    if (sessionList[socketIndex] == NULL)
    {
        return;
    }

    int shutdownResult = shutdown(
        sessionList[socketIndex]->socketDescriptor,
        SD_BOTH
    );

    if (shutdownResult != SOCKET_ERROR)
    {
        printf("\tSocket was shutdown!\n");
    }
    else
    {
        printf("\tSocket shutdown error. Error code: %d!\n", WSAGetLastError());
    }


    int closesocketResult = closesocket(
        sessionList[socketIndex]->socketDescriptor
    );

    if (closesocketResult != SOCKET_ERROR)
    {
        printf("\tSocket was closed!\n");
    }
    else
    {
        printf("\tSocket close error. Error code: %d!\n", WSAGetLastError());
    }


    DestroySession(socketIndex);


    socketCount--;
}

void SocketResolveAddress(LPSOCKADDR p_remoteAddress, DWORD remoteAddressLength)
{
    WCHAR remoteIpString[REMOTE_ADDRESS_STRING_MAX_SIZE + 1] = {0};
    // + 1 is for the null terminator
    DWORD remoteIpStringSize = REMOTE_ADDRESS_STRING_MAX_SIZE;
    // we copy REMOTE_ADDRESS_STRING_MAX_SIZE * sizeof(WCHAR)
    // bytes of information + NULL char
    // WSAAddressToStringW also copies port number into the
    // remoteIpString so REMOTE_ADDRESS_STRING_MAX_SIZE is
    // REMOTE_IP_STRING_MAX_SIZE + 1 (: symbol) +
    // REMOTE_PORT_STRING_MAX_SIZE this gives us 15 + 1 + 5 =
    // 21 symbols

    if (WSAAddressToStringW(
            p_remoteAddress,
            remoteAddressLength,
            NULL,
            (LPWSTR)&remoteIpString,
            &remoteIpStringSize
        )
        == SOCKET_ERROR)
    {
        printf(
            "\tCouldn't convert remote IP to string. "
            "Error code: %d\n",
            WSAGetLastError()
        );
    }
    else
    {
        wprintf(L"\tRemote host: %s\n", remoteIpString);
    }
}

void Cleanup()
{
    for (int socketIndex = 0; socketIndex < socketCount; socketIndex++)
    {
        printf("Socket %d:\n", socketIndex);
        SocketClose(socketIndex);
    }


    WSACleanup();
}
