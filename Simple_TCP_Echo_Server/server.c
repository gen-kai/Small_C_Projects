#include <stdbool.h>
#include <stdio.h>
#include <winsock2.h>
#include <ws2tcpip.h>

#pragma comment(lib, "ws2_32.lib")

#define LISTEN_SOCKET_COUNT 1U
#define SOCKET_POLL_TIMEOUT 10U
#define LISTEN_SOCKET_PORT 62390U
#define MESSAGE_MAXIMUM_LENGTH 100U

bool IsSocketReady(WSAPOLLFD* saPollFdStructure);
int ConnectionLoop(SOCKET listenSocket, SOCKET connectionSocket);

int main(int argCount, char* argValues[])
{
    WORD wsaVersion = MAKEWORD(2, 2);
    WSADATA wsaData;

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


    SOCKET listenSocket = socket
    (
        AF_INET,
        SOCK_STREAM,
        IPPROTO_TCP
    );
    if (listenSocket == INVALID_SOCKET)
    {
        printf("Socket creation failed with error %d\n", WSAGetLastError());
        WSACleanup();
        return 3;
    }


    printf("Unnamed socket was created!\n");


    /*    WSAPOLLFD wsaPollFdStructure =
        {
            .fd = listenSocket,
            .events = POLLRDNORM | POLLWRNORM,
            .revents = 0
        };

        int socketStatus = WSAPoll
        (
            &wsaPollFdStructure,
            LISTEN_SOCKET_COUNT,
            SOCKET_POLL_TIMEOUT
        );
        if (socketStatus == INVALID_SOCKET)
        {
            printf("Socket poll was unsuccessful. Error code: %d\n", WSAGetLastError());


            int closesocketResult = closesocket(listenSocket);
            if (closesocketResult == SOCKET_ERROR)
            {
                printf("Socket close was unsuccessful. Error code: %d\n", WSAGetLastError());
            }


            WSACleanup();
            return 4;
        }
        else if (socketStatus != LISTEN_SOCKET_COUNT)
        {
            printf("Socket poll timeout!\n");


            int closesocketResult = closesocket(listenSocket);
            if (closesocketResult == SOCKET_ERROR)
            {
                printf("Socket close was unsuccessful. Error code: %d\n", WSAGetLastError());
            }


            WSACleanup();
            return 5;
        }


        printf("Socket availability was polled!\n");


        if (!IsSocketReady(&wsaPollFdStructure))
        {
            printf("Socket is not ready!\n");


            int closesocketResult = closesocket(listenSocket);
            if (closesocketResult == SOCKET_ERROR)
            {
                printf("Socket close was unsuccessful. Error code: %d\n", WSAGetLastError());
            }


            WSACleanup();
            return 6;
        }*/


    printf("Socket is ready!\n");


    SOCKADDR_IN sockaddrStructure =
    {
        .sin_family = AF_INET,
        .sin_port = htons(LISTEN_SOCKET_PORT),
        .sin_addr = INADDR_ANY,
        .sin_zero = 0
    };

    int bindResult = bind
    (
        listenSocket,
        &sockaddrStructure,
        sizeof(sockaddrStructure)
    );
    if (bindResult == SOCKET_ERROR)
    {
        printf("Socket bind was unsuccessfull. Error code: %d\n", WSAGetLastError());


        int closesocketResult = closesocket(listenSocket);
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
        listenSocket,
        SOMAXCONN_HINT(1)
    );
    if (listenResult == SOCKET_ERROR)
    {
        printf("Socket listen was unsuccessfull. Error code: %d\n", WSAGetLastError());


        int closesocketResult = closesocket(listenSocket);
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
        SOCKADDR remoteAddress;
        int remoteAddressLength = sizeof(remoteAddress);

        SOCKET connectionSocket = accept
        (
            listenSocket,
            &remoteAddress,
            &remoteAddressLength
        );
        if (connectionSocket == INVALID_SOCKET)
        {
            printf("Connection accept was unsuccessfull. Error code: %d\n", WSAGetLastError());


            int closesocketResult = closesocket(listenSocket);
            if (closesocketResult == SOCKET_ERROR)
            {
                printf("Socket close was unsuccessful. Error code: %d\n", WSAGetLastError());
            }


            WSACleanup();
            return 9;
        }
        else
        {
            printf("New connection was established!\n");


            char remoteIpString[15];

            if (inet_ntop
            (
                AF_INET,
                &remoteAddress.sa_data[2],
                &remoteIpString,
                sizeof(remoteIpString)
            ) == NULL)
            {
                printf("Couldn't convert remote IP to string!\n");
            }
            else
            {
                printf("Established connection with the host %s\n", remoteIpString);
            }
        }


        int echoLoopResult = ConnectionLoop(listenSocket, connectionSocket);
        if (echoLoopResult == 10)
        {
            printf("recv failed: %d\n", WSAGetLastError());


            int closesocketResult = closesocket(listenSocket);
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
        else if (echoLoopResult == 11)
        {
            printf("recv failed: %d\n", WSAGetLastError());


            int closesocketResult = closesocket(listenSocket);
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


    int closesocketResult = closesocket(listenSocket);
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

bool IsSocketReady(WSAPOLLFD* p_wsaPollFdStructure)
{
    bool isSocketReadyToRead = (p_wsaPollFdStructure->revents | POLLRDNORM) == POLLRDNORM;
    // if bitwise AND of revents and POLLRDNORM is True - isSocketReadyToRead is True
    bool isSocketReadyToWrite = (p_wsaPollFdStructure->revents | POLLWRNORM) == POLLWRNORM;
    // if bitwise AND of revents and POLLWRNORM is True - isSocketReadyToWrite is True

    return isSocketReadyToRead && isSocketReadyToWrite;
    // the function returns True only if both isSocketReadyToRead and isSocketReadyToWrite are TRUE
}

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
}