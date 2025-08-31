#include <stdbool.h>
#include <stdio.h>
#include <winsock2.h>
#include "server.h"


#pragma comment(lib, "ws2_32.lib")


int main(int argCount, char* argValues[])
{
    WORD wsaVersion = MAKEWORD(2, 2);
    WSADATA wsaData;
    SOCKET listeningSocket = INVALID_SOCKET;
    SOCKADDR_IN socketAddress =
    {
        .sin_family = SOCKET_FAMILY,
        .sin_port = htons(SOCKET_PORT),
        .sin_addr.s_addr = SOCKET_ADDRESS
    };
    SOCKADDR remoteAddress;
    int remoteAddressLength = sizeof(remoteAddress);


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


    listeningSocket = socket
    (
        SOCKET_FAMILY,
        SOCKET_TYPE,
        SOCKET_PROTOCOL
    );
    if (listeningSocket == INVALID_SOCKET)
    {
        printf("Socket creation failed with error %d\n", WSAGetLastError());
        WSACleanup();
        return 3;
    }


    printf("Unnamed socket was created!\n");


    int bindResult = bind
    (
        listeningSocket,
        &socketAddress,
        sizeof(socketAddress)
    );
    if (bindResult == SOCKET_ERROR)
    {
        printf("Socket bind was unsuccessfull. Error code: %d\n", WSAGetLastError());


        int closesocketResult = closesocket(listeningSocket);
        if (closesocketResult == SOCKET_ERROR)
        {
            printf("Socket close was unsuccessful. Error code: %d\n", WSAGetLastError());
        }


        WSACleanup();
        return 7;
    }


    printf("Socket was bound!\n");


    int listenResult = listen
    (
        listeningSocket,
        SOMAXCONN_HINT(SOCKET_MAXIMUM_CONNECTIONS)
    );
    if (listenResult == SOCKET_ERROR)
    {
        printf("Socket listen was unsuccessfull. Error code: %d\n", WSAGetLastError());


        int closesocketResult = closesocket(listeningSocket);
        if (closesocketResult == SOCKET_ERROR)
        {
            printf("Socket close was unsuccessful. Error code: %d\n", WSAGetLastError());
        }


        WSACleanup();
        return 8;
    }


    printf("Socket was switched into listen state!\n");


    while (true)
    {
        SOCKET connectionSocket = accept
        (
            listeningSocket,
            &remoteAddress,
            &remoteAddressLength
        );
        if (connectionSocket == INVALID_SOCKET)
        {
            printf("Connection accept was unsuccessfull. Error code: %d\n", WSAGetLastError());


            int closesocketResult = closesocket(listeningSocket);
            if (closesocketResult == SOCKET_ERROR)
            {
                printf("Socket close was unsuccessful. Error code: %d\n", WSAGetLastError());
            }


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
            // WSAAddressToStringW also copies port number into the remoteIpString
            // so REMOTE_ADDRESS_STRING_MAX_SIZE is
            // REMOTE_IP_STRING_MAX_SIZE + 1 (: symbol) + REMOTE_PORT_STRING_MAX_SIZE
            // this gives us 15 + 1 + 5 = 21 symbols

            if (WSAAddressToStringW
            (
                &remoteAddress,
                (DWORD) remoteAddressLength,
                NULL,
                (LPWSTR) &remoteIpString,
                &remoteIpStringSize
            ) == SOCKET_ERROR)
            {
                printf
                (
                    "Couldn't convert remote IP to string! "
                    "Error code: %d\n", WSAGetLastError()
                );
            }
            else
            {
                wprintf(L"Remote host: %s\n", remoteIpString);
            }
        }


        int connectionLoopResult = ConnectionLoop(listeningSocket, connectionSocket);
        if (connectionLoopResult == 10)
        {
            printf("recv failed: %d\n", WSAGetLastError());


            int closesocketResult = closesocket(listeningSocket);
            if (closesocketResult == SOCKET_ERROR)
            {
                printf
                (
                    "Listen socket close was unsuccessful. Error code: %d\n",
                    WSAGetLastError()
                );
            }


            closesocketResult = closesocket(connectionSocket);
            if (closesocketResult == SOCKET_ERROR)
            {
                printf
                (
                    "Listen socket close was unsuccessful. Error code: %d\n",
                    WSAGetLastError()
                );
            }


            WSACleanup();
            return 10;
        }
        else if (connectionLoopResult == 11)
        {
            printf("recv failed: %d\n", WSAGetLastError());


            int closesocketResult = closesocket(listeningSocket);
            if (closesocketResult == SOCKET_ERROR)
            {
                printf
                (
                    "Listen socket close was unsuccessful. Error code: %d\n",
                    WSAGetLastError()
                );
            }


            closesocketResult = closesocket(connectionSocket);
            if (closesocketResult == SOCKET_ERROR)
            {
                printf
                (
                    "Listen socket close was unsuccessful. Error code: %d\n",
                    WSAGetLastError()
                );
            }


            WSACleanup();
            return 11;
        }
    };


    int closesocketResult = closesocket(listeningSocket);
    if (closesocketResult == SOCKET_ERROR)
    {
        printf("Socket close was unsuccessful. Error code: %d\n", WSAGetLastError());
        WSACleanup();
        return -1;
    }


    printf("Socket was closed!\n");


    WSACleanup();
    return 0;
};


int ConnectionLoop(SOCKET listenSocket, SOCKET connectionSocket)
{
    char receivedMessage[MESSAGE_MAXIMUM_LENGTH] = {0};
    int bytesReceived = 0;

    do
    {
        bytesReceived = recv
        (
            connectionSocket,
            receivedMessage,
            MESSAGE_MAXIMUM_LENGTH,
            0
        );
        if (bytesReceived > 0)
        {
            printf("Bytes received: %d\n", bytesReceived);


            int bytesSent = send
            (
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
                printf("Connection closed\n");
                return 0;
            }
            else
            {
                return 10;
            }
        }
        else if (bytesReceived == 0)
        {
            printf("Connection closed\n");
            return 0;
        }
        else
        {
            return 11;
        }
    }
    while (bytesReceived > 0);


    return 0;
}