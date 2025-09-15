#include <stdio.h>
#include <winsock2.h>
#include "server.h"


int            socketCount = 0;
WSAEVENT       eventList[WSA_MAXIMUM_WAIT_EVENTS];
SOCKET_OBJECT* socketList[WSA_MAXIMUM_WAIT_EVENTS];

int main(int argCount, char* argValues[])
{
    WORD    wsaVersion = MAKEWORD(2, 2);
    WSADATA wsaData;

    SOCKET      listeningSocket        = INVALID_SOCKET;
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


        return 1;
    }

    if (LOBYTE(wsaData.wVersion) != 2 || HIBYTE(wsaData.wVersion) != 2)
    {
        printf("Could not find a usable version of Winsock.dll\n");


        WSACleanup();
        return 2;
    }
    else
    {
        printf(
            "Winsock.dll of version %d.%d was loaded\n",
            LOBYTE(wsaData.wVersion),
            HIBYTE(wsaData.wVersion)
        );
    }


    listeningSocket = socket(
        LISTENING_SOCKET_FAMILY,
        LISTENING_SOCKET_TYPE,
        LISTENING_SOCKET_PROTOCOL
    );

    if (listeningSocket == INVALID_SOCKET)
    {
        printf("Socket creation failed with error %d\n", WSAGetLastError());


        WSACleanup();
        return 3;
    }
    else
    {
        printf("Unnamed socket was created!\n");
    }


    int createSocketObjectResult = CreateSocketObject(listeningSocket);

    if (createSocketObjectResult == 4)
    {
        printf("Unnamed socket:\n");


        int shutdownResult = shutdown(listeningSocket, SD_BOTH);

        if (shutdownResult != SOCKET_ERROR)
        {
            printf("    Socket was shutdown!\n");
        }
        else
        {
            printf(
                "    Socket shutdown error. Error code: %d!\n",
                WSAGetLastError()
            );
        }


        int closesocketResult = closesocket(listeningSocket);

        if (closesocketResult != SOCKET_ERROR)
        {
            printf("    Socket was closed!\n");
        }
        else
        {
            printf(
                "    Socket close error. Error code: %d!\n",
                WSAGetLastError()
            );
        }


        WSACleanup();
        return 4;
    }
    else if (createSocketObjectResult == 5)
    {
        printf("Unnamed socket:\n");


        int shutdownResult = shutdown(listeningSocket, SD_BOTH);

        if (shutdownResult != SOCKET_ERROR)
        {
            printf("    Socket was shutdown!\n");
        }
        else
        {
            printf(
                "    Socket shutdown error. Error code: %d!\n",
                WSAGetLastError()
            );
        }


        int closesocketResult = closesocket(listeningSocket);

        if (closesocketResult != SOCKET_ERROR)
        {
            printf("    Socket was closed!\n");
        }
        else
        {
            printf(
                "    Socket close error. Error code: %d!\n",
                WSAGetLastError()
            );
        }


        WSACleanup();
        return 5;
    }
    else
    {
        printf("Unnamed socket information object was created!\n");
    }


    int bindResult = bind(
        listeningSocket,
        (SOCKADDR*)&listeningSocketAddress,
        sizeof(listeningSocketAddress)
    );

    if (bindResult == SOCKET_ERROR)
    {
        printf(
            "Socket bind was unsuccessfull. Error code: %d\n",
            WSAGetLastError()
        );


        printf("Listening socket:\n");
        SocketDispose(socketCount - 1);


        WSACleanup();
        return 6;
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


        printf("Listening socket:\n");
        SocketDispose(socketCount - 1);


        WSACleanup();
        return 7;
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
            "Registering Accept Event failed with error %d\n",
            WSAGetLastError()
        );

        printf("Listening socket:\n");
        SocketDispose(socketCount - 1);


        WSACleanup();
        return 8;
    }
    else
    {
        printf("Accept Event was registered!\n");
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
                SocketDispose(socketIndex);
            }


            WSACleanup();
            return 9;
        }
        else
        {
            eventIndex -= WSA_WAIT_EVENT_0;


            printf("Socket %d signalled back!\n", eventIndex);
        }


        int wsaEnumNetworkEventsResult = WSAEnumNetworkEvents(
            socketList[eventIndex]->socketDescriptor,
            eventList[eventIndex],
            &networkEvents
        );

        if (wsaEnumNetworkEventsResult == SOCKET_ERROR)
        {
            printf(
                "WSAEnumNetworkEvents() failed with error %d\n",
                WSAGetLastError()
            );


            for (int socketIndex = 0; socketIndex < socketCount; socketIndex++)
            {
                printf("Socket %d:\n", socketIndex);
                SocketDispose(socketIndex);
            }


            WSACleanup();
            return 10;
        }
        else
        {
            printf("Enumerated network events for socket %d!\n", eventIndex);
        }


        // process FD_ACCEPT vs. FD_READ | FD_WRITE vs. FD_CLOSE events
        // process FD_ACCEPT
        if (networkEvents.lNetworkEvents & FD_ACCEPT)
        {
            if (networkEvents.iErrorCode[FD_ACCEPT_BIT] != 0)
            {
                printf(
                    "FD_ACCEPT failed with error %d\n",
                    networkEvents.iErrorCode[FD_ACCEPT_BIT]
                );


                break;
            }


            int acceptResult = SocketAccept(eventIndex);

            if (acceptResult != 0)
            {
                continue;
            }
        }
        // process FD_READ
        else if (networkEvents.lNetworkEvents & FD_READ)
        {
            if (networkEvents.iErrorCode[FD_READ_BIT] != 0)
            {
                printf(
                    "FD_READ failed with error %d\n",
                    networkEvents.iErrorCode[FD_READ_BIT]
                );


                // exit the loop and clean up
                break;
            }
            else
            {
                printf("FD_READ was successful!\n");
            }

            int readResult = SocketRead(eventIndex);

            if (readResult != 0)
            {
                // exit the loop and clean up
                break;
            }
        }
        // process FD_WRITE
        else if (networkEvents.lNetworkEvents & FD_WRITE)
        {
            if (networkEvents.iErrorCode[FD_WRITE_BIT] != 0)
            {
                printf(
                    "FD_WRITE failed with error %d\n",
                    networkEvents.iErrorCode[FD_WRITE_BIT]
                );


                // exit the loop and clean up
                break;
            }
            else
            {
                printf("FD_WRITE was successful!\n");
            }


            int writeResult = SocketWrite(eventIndex);

            if (writeResult != 0)
            {
                // exit the loop and clean up
                break;
            }
        }
        // process FD_CLOSE
        else if (networkEvents.lNetworkEvents & FD_CLOSE)
        {
            SocketDispose(eventIndex);
        }
    };


    for (int socketIndex = 0; socketIndex < socketCount; socketIndex++)
    {
        printf("Socket %d:\n", socketIndex);
        SocketDispose(socketIndex);
    }


    WSACleanup();
    return 0;
}

int CreateSocketObject(SOCKET socketDescriptor)
{
    WSAEVENT newWSAEvent = WSACreateEvent();

    if (newWSAEvent == WSA_INVALID_EVENT)
    {
        printf(
            "Event creation for socket failed with error %d\n",
            WSAGetLastError()
        );


        return 4;
    }

    eventList[socketCount] = newWSAEvent;


    SOCKET_OBJECT* p_socketObject = HeapAlloc(
        GetProcessHeap(),
        HEAP_ZERO_MEMORY,
        sizeof(SOCKET_OBJECT)
    );

    if (p_socketObject == NULL)
    {
        printf("Creation of the socket object failed!\n");


        return 5;
    }


    p_socketObject->socketDescriptor        = socketDescriptor;
    p_socketObject->dataBufferBytesOccupied = 0;
    p_socketObject->wsaBufStructure.buf     = p_socketObject->dataBuffer;
    p_socketObject->wsaBufStructure.len     = DEFAULT_BUFFER_SIZE;


    socketList[socketCount] = p_socketObject;
    socketCount++;
    return 0;
}

void SocketDispose(int socketIndex)
{
    int shutdownResult = shutdown(
        socketList[socketIndex]->socketDescriptor,
        SD_BOTH
    );

    if (shutdownResult != SOCKET_ERROR)
    {
        printf("    Socket was shutdown!\n");
    }
    else
    {
        printf(
            "    Socket shutdown error. Error code: %d!\n",
            WSAGetLastError()
        );
    }


    int closesocketResult = closesocket(
        socketList[socketIndex]->socketDescriptor
    );

    if (closesocketResult != SOCKET_ERROR)
    {
        printf("    Socket was closed!\n");
    }
    else
    {
        printf("    Socket close error. Error code: %d!\n", WSAGetLastError());
    }


    FreeSocketObject(socketIndex);


    socketCount--;
}

void FreeSocketObject(int socketIndex)
{
    HeapFree(GetProcessHeap(), 0, socketList[socketCount]);


    if (WSACloseEvent(eventList[socketIndex]) == TRUE)
    {
        printf("WSACloseEvent() is OK!\n\n");
    }
    else
    {
        printf("WSACloseEvent() failed!\n\n");
    }


    for (int elementIndex = socketIndex; elementIndex < socketCount;
         elementIndex++)
    {
        eventList[elementIndex] = eventList[elementIndex + 1];

        socketList[elementIndex] = socketList[elementIndex + 1];
    }
}

int SocketAccept(int eventIndex)
{
    SOCKADDR remoteAddress;
    int      remoteAddressLength = sizeof(remoteAddress);


    if (socketCount >= WSA_MAXIMUM_WAIT_EVENTS)
    {
        printf("Too many connections, continue with the next event!\n");

        return 0;
    }


    SOCKET connectionSocket = accept(
        socketList[eventIndex]->socketDescriptor,
        &remoteAddress,
        &remoteAddressLength
    );

    if (connectionSocket == INVALID_SOCKET)
    {
        printf(
            "Connection accept on socket %d was unsuccessfull. "
            "Error code: %d\n",
            eventIndex,
            WSAGetLastError()
        );


        return 1;
    }
    else
    {
        printf("New connection established!\n");


        SocketResolveAddress(&remoteAddress, remoteAddressLength);
    }


    int createSocketObjectResult = CreateSocketObject(connectionSocket);

    if (createSocketObjectResult == 4)
    {
        printf("Connection socket:\n");


        int shutdownResult = shutdown(connectionSocket, SD_BOTH);

        if (shutdownResult != SOCKET_ERROR)
        {
            printf("    Socket was shutdown!\n");
        }
        else
        {
            printf(
                "    Socket shutdown error. Error code: %d!\n",
                WSAGetLastError()
            );
        }


        int closesocketResult = closesocket(connectionSocket);

        if (closesocketResult != SOCKET_ERROR)
        {
            printf("    Socket was closed!\n");
        }
        else
        {
            printf(
                "    Socket close error. Error code: %d!\n",
                WSAGetLastError()
            );
        }


        return 2;
    }
    else if (createSocketObjectResult == 5)
    {
        printf("Connection socket:\n");


        int shutdownResult = shutdown(connectionSocket, SD_BOTH);

        if (shutdownResult != SOCKET_ERROR)
        {
            printf("    Socket was shutdown!\n");
        }
        else
        {
            printf(
                "    Socket shutdown error. Error code: %d!\n",
                WSAGetLastError()
            );
        }


        int closesocketResult = closesocket(connectionSocket);

        if (closesocketResult != SOCKET_ERROR)
        {
            printf("    Socket was closed!\n");
        }
        else
        {
            printf(
                "    Socket close error. Error code: %d!\n",
                WSAGetLastError()
            );
        }


        return 3;
    }

    printf("Connection socket information object was created!\n");


    int registerConnectionSocket = WSAEventSelect(
        socketList[socketCount - 1]->socketDescriptor,
        eventList[socketCount - 1],
        FD_WRITE | FD_CLOSE
    );

    if (registerConnectionSocket == SOCKET_ERROR)
    {
        printf(
            "Registering connection socket failed with error %d\n",
            WSAGetLastError()
        );

        printf("Listening socket:\n");
        SocketDispose(socketCount - 1);


        return 4;
    }
    else
    {
        printf("Connection socket was registered!\n");
    }


    return 0;
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
            "Couldn't convert remote IP to string. "
            "Error code: %d\n",
            WSAGetLastError()
        );
    }
    else
    {
        wprintf(L"Remote host: %s\n", remoteIpString);
    }
}

// Receive from socket, select FD_READ, send to socket
int SocketRead(int eventIndex)
{
    // READ from socket, set buffer bytes occupied, select FD_WRITE
    DWORD recvFlags = 0;
    if (socketList[eventIndex]->dataBufferBytesOccupied == 0)
    {
        int wsaRecvResult = WSARecv(
            socketList[eventIndex]->socketDescriptor,
            &(socketList[eventIndex]->wsaBufStructure),
            1,
            &(socketList[eventIndex]->dataBufferBytesOccupied),
            &recvFlags,
            NULL,
            NULL
        );

        if (wsaRecvResult == SOCKET_ERROR)
        {
            printf("WSARecv() failed with error %d\n", WSAGetLastError());


            return 1;
        }
        else
        {
            printf("WSARecv() was successful!\n");
        }

        // Set WSABUF length to the number of bytes recieved
        socketList[eventIndex]->wsaBufStructure.len =
            socketList[eventIndex]->dataBufferBytesOccupied;


        int registerWriteEvent = WSAEventSelect(
            socketList[eventIndex]->socketDescriptor,
            eventList[eventIndex],
            FD_READ | FD_CLOSE
        );

        if (registerWriteEvent == SOCKET_ERROR)
        {
            printf(
                "Registering FD_WRITE event after FD_READ failed with "
                "error %d\n",
                WSAGetLastError()
            );


            return 2;
        }
        else
        {
            printf("The next event after FD_READ was registered!\n");
        }
    }


    if (SocketWrite(eventIndex) != 0)
    {
        return 3;
    }


    return 0;
}

// Send buffer to socket, reset bytes occupied, select FD_READ if the entire
// buffer was sent
int SocketWrite(int eventIndex)
{
    DWORD sendFlags               = 0;
    DWORD bytesSent               = 0;
    long  networkEventsToRegister = 0;

    if (socketList[eventIndex]->dataBufferBytesOccupied != 0)
    {
        int wsaSendResult = WSASend(
            socketList[eventIndex]->socketDescriptor,
            &(socketList[eventIndex]->wsaBufStructure),
            1,
            &bytesSent,
            sendFlags,
            NULL,
            NULL
        );

        if ((wsaSendResult == SOCKET_ERROR)
            && (WSAGetLastError() != WSAEWOULDBLOCK))
        {
            printf("WSASend() failed with error %d\n", WSAGetLastError());
        }
        else if (WSAGetLastError() == WSAEWOULDBLOCK)
        {
            printf(
                "WSASend() returned WSAEWOULDBLOCK. The send operation cannot "
                "be completed immediately! Rescheduling..."
            );
        }
        else
        {
            printf("WSASend() was successful!\n");
        }


        socketList[eventIndex]->dataBufferBytesOccupied -= bytesSent;


        // if we didn't send the whole buffer - register new write
        // event, otherwise - read and set WSABUF length to the number
        // of bytes left after WSASend
        if ((socketList[eventIndex]->dataBufferBytesOccupied != 0)
            || (WSAGetLastError() == WSAEWOULDBLOCK))
        {
            networkEventsToRegister = FD_WRITE | FD_CLOSE;
            socketList[eventIndex]->wsaBufStructure.len =
                socketList[eventIndex]->dataBufferBytesOccupied;
        }
        else
        {
            networkEventsToRegister                     = FD_READ | FD_CLOSE;
            socketList[eventIndex]->wsaBufStructure.len = DEFAULT_BUFFER_SIZE;
        }

        int registerNextEvent = WSAEventSelect(
            socketList[eventIndex]->socketDescriptor,
            eventList[eventIndex],
            networkEventsToRegister
        );

        if (registerNextEvent == SOCKET_ERROR)
        {
            printf(
                "Registering the next event after FD_WRITE failed with "
                "error %d\n",
                WSAGetLastError()
            );


            return 2;
        }
        else
        {
            if (networkEventsToRegister & FD_READ == FD_READ)
            {
                printf("FD_READ event after FD_WRITE was registered!\n");
            }
            else
            {
                printf("FD_WRITE event after FD_WRITE was registered!\n");
            }
        }
    }
    else
    {
        int registerNextEvent = WSAEventSelect(
            socketList[eventIndex]->socketDescriptor,
            eventList[eventIndex],
            FD_READ | FD_CLOSE
        );

        if (registerNextEvent == SOCKET_ERROR)
        {
            printf(
                "Registering FD_READ event after FD_WRITE failed with "
                "error %d\n",
                WSAGetLastError()
            );


            return 3;
        }
        else
        {
            printf("FD_READ event after FD_WRITE was registered!\n");
        }
    }


    return 0;
}
