#include <stdio.h>
#include <winsock2.h>
#include "server.h"


#pragma comment(lib, "ws2_32.lib")

int main(int argCount, char* argValues[])
{
    WORD    wsaVersion = MAKEWORD(2, 2);
    WSADATA wsaData;
    SOCKET  listeningSocket = INVALID_SOCKET;

    SOCKADDR_IN socketAddress = {
        .sin_family      = LISTENING_SOCKET_FAMILY,
        .sin_port        = htons(LISTENING_SOCKET_PORT),
        .sin_addr.s_addr = LISTENING_SOCKET_ADDRESS
    };

    SOCKADDR remoteAddress;
    int      remoteAddressLength = sizeof(remoteAddress);


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


    printf("Unnamed socket was created!\n");


    int bindResult = bind(
        listeningSocket,
        (SOCKADDR*)&socketAddress,
        sizeof(socketAddress)
    );

    if (bindResult == SOCKET_ERROR)
    {
        printf(
            "Socket bind was unsuccessfull. Error code: %d\n",
            WSAGetLastError()
        );


        printf("Listening socket:\n");
        SocketClose(listeningSocket);


        WSACleanup();
        return 7;
    }


    printf("Socket was bound!\n");


    int listenResult = listen(
        listeningSocket,
        SOMAXCONN_HINT(SOCKET_MAXIMUM_CONNECTIONS)
    );

    if (listenResult == SOCKET_ERROR)
    {
        printf(
            "Socket listen was unsuccessfull. Error code: %d\n",
            WSAGetLastError()
        );


        printf("Listening socket:\n");
        SocketClose(listeningSocket);


        WSACleanup();
        return 8;
    }


    printf("Socket was switched into listen state!\n");


    while (TRUE)
    {
        SOCKET connectionSocket = accept(
            listeningSocket,
            &remoteAddress,
            &remoteAddressLength
        );

        if (connectionSocket == INVALID_SOCKET)
        {
            printf(
                "Connection accept was unsuccessfull. Error code: "
                "%d\n",
                WSAGetLastError()
            );


            printf("Listening socket:\n");
            SocketClose(listeningSocket);


            WSACleanup();
            return 9;
        }
        else
        {
            printf("New connection established!\n");


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
                    &remoteAddress,
                    (DWORD)remoteAddressLength,
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


        int connectionLoopResult = ConnectionLoop(connectionSocket);

        if (connectionLoopResult == 10)
        {
            printf("recv failed: %d\n", WSAGetLastError());


            printf("Connection socket:\n");
            SocketClose(connectionSocket);


            continue;
        }
        else if (connectionLoopResult == 11)
        {
            printf("send failed: %d\n", WSAGetLastError());


            printf("Connection socket:\n");
            SocketClose(connectionSocket);


            continue;
        }
        else if (connectionLoopResult == 0)
        {
            printf("Connection socket:\n");
            SocketClose(connectionSocket);


            continue;
        }
    };


    printf("Listening socket:\n");
    SocketClose(listeningSocket);


    WSACleanup();
    return 0;
};

int ConnectionLoop(SOCKET connectionSocket)
{
    char receivedMessage[MESSAGE_MAXIMUM_LENGTH] = {0};
    int  bytesReceived                           = 0;

    do
    {
        bytesReceived = recv(
            connectionSocket,
            receivedMessage,
            MESSAGE_MAXIMUM_LENGTH,
            0
        );

        if (bytesReceived > 0)
        {
            printf("Bytes received: %d\n", bytesReceived);


            int bytesSent = send(
                connectionSocket,
                receivedMessage,
                bytesReceived,
                0
            );
            if (bytesSent == bytesReceived)
            {
                printf("Bytes sent: %d\n", bytesSent);
            }
            else if (bytesReceived == 0)
            {
                printf("Connection closed!\n");
                return 0;
            }
            else
            {
                return 11;
            }
        }
        else if (bytesReceived == 0)
        {
            printf("Connection closed!\n");
            return 0;
        }
        else
        {
            return 10;
        }
    }
    while (bytesReceived > 0);


    return 0;
}

int SocketClose(SOCKET socketToDispose)
{
    BOOL isSocketShut   = FALSE;
    BOOL isSocketClosed = FALSE;

    int shutdownResult = shutdown(socketToDispose, SD_BOTH);

    if (shutdownResult != SOCKET_ERROR)
    {
        printf("    Socket was shutdown!\n");
        isSocketShut = TRUE;
    }
    else
    {
        printf(
            "    Socket shutdown error. Error code: %d!\n",
            WSAGetLastError()
        );
    }


    int closesocketResult = closesocket(socketToDispose);

    if (closesocketResult != SOCKET_ERROR)
    {
        printf("    Socket was closed!\n");
        isSocketClosed = TRUE;
    }
    else
    {
        printf("    Socket close error. Error code: %d!\n", WSAGetLastError());
    }


    if (isSocketShut && !isSocketClosed)
    {
        return 1;
    }
    else if (!isSocketShut && isSocketClosed)
    {
        return 2;
    }
    else if (!isSocketShut && !isSocketClosed)
    {
        return 3;
    }
    else if (isSocketShut && isSocketClosed)
    {
        return 0;
    }
}
