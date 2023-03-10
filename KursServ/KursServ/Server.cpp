#undef UNICODE

#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdio.h>
#include <fstream>

#include <iostream>

// Need to link with Ws2_32.lib
#pragma comment (lib, "Ws2_32.lib")
// #pragma comment (lib, "Mswsock.lib")

#define DEFAULT_BUFLEN 512
#define DEFAULT_PORT "27015"

int __cdecl main(void)
{
    WSADATA wsaData;
    int iResult;

    SOCKET ListenSocket = INVALID_SOCKET;
    SOCKET ClientSocket = INVALID_SOCKET;

    struct addrinfo* result = NULL;
    struct addrinfo hints;

    int iSendResult;
    char recvbuf[DEFAULT_BUFLEN];
    int recvbuflen = DEFAULT_BUFLEN;
    bool menu = true;
    std::cout << "Kosheliuk Maksim \n";
    std::cout << "Student of group BKI-19 \n";
    std::cout << "Version 10 \n";
    // Initialize Winsock
    iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != 0) {
        printf("WSAStartup failed with error: %d\n", iResult);
        return 1;
    }
    int sizehints = sizeof(hints);
    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_flags = AI_PASSIVE;

    // Resolve the server address and port
    iResult = getaddrinfo(NULL, DEFAULT_PORT, &hints, &result);
    if (iResult != 0) {
        printf("getaddrinfo failed with error: %d\n", iResult);
        WSACleanup();
        return 1;
    }

    // Create a SOCKET for the server to listen for client connections.
    ListenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
    if (ListenSocket == INVALID_SOCKET) {
        printf("socket failed with error: %ld\n", WSAGetLastError());
        freeaddrinfo(result);
        WSACleanup();
        return 1;
    }

    // Setup the TCP listening socket
    iResult = bind(ListenSocket, result->ai_addr, (int)result->ai_addrlen);
    if (iResult == SOCKET_ERROR) {
        printf("bind failed with error: %d\n", WSAGetLastError());
        freeaddrinfo(result);
        closesocket(ListenSocket);
        WSACleanup();
        return 1;
    }

    freeaddrinfo(result);

    iResult = listen(ListenSocket, SOMAXCONN);
    if (iResult == SOCKET_ERROR) {
        printf("listen failed with error: %d\n", WSAGetLastError());
        closesocket(ListenSocket);
        WSACleanup();
        return 1;
    }

    // Accept a client socket
    ClientSocket = accept(ListenSocket, NULL, NULL);
    if (ClientSocket == INVALID_SOCKET) {
        printf("accept failed with error: %d\n", WSAGetLastError());
        closesocket(ListenSocket);
        WSACleanup();
        return 1;
    }

    // No longer need server socket
    closesocket(ListenSocket);

    std::cout << "Conected!\n";
    int key;
    while (menu)
    {
        recv(ClientSocket, (char*)&key, sizeof(key), 0);
        int k = 0;
        const int BUFFER_SIZE = 1024;
        char bufferFile[BUFFER_SIZE];
        char fileRequested[FILENAME_MAX];
        int byRecv;
        std::ofstream file;
        while (key == 1)
        {
            if (key == 1)std::cout << "File sending...\n";
          //  if (k == 0)send(ConnectSocket, (char*)&key, sizeof(key), 0);

            bool clientClose = false;
            int codeAvailable = 404;
            const int fileAvailable = 200;
            const int fileNotfound = 404;
            long fileRequestedsize = 0;

            do {
                int fileDownloaded = 0;
                memset(fileRequested, 0, FILENAME_MAX);
                int byRecv = recv(ClientSocket, fileRequested, FILENAME_MAX, 0);
                if (byRecv == 0 || byRecv == -1) {
                    clientClose = true;
                }

                byRecv = recv(ClientSocket, (char*)&codeAvailable, sizeof(int), 0);
                if (byRecv == 0 || byRecv == -1) {
                    clientClose = true;
                    break;
                }
                if (codeAvailable == 200) {
                    byRecv = recv(ClientSocket, (char*)&fileRequestedsize, sizeof(long), 0);
                    if (byRecv == 0 || byRecv == -1) {
                        clientClose = true;
                        break;
                    }
                    file.open(fileRequested, std::ios::binary | std::ios::trunc);
                    do {
                        memset(bufferFile, 0, BUFFER_SIZE);
                        byRecv = recv(ClientSocket, bufferFile, BUFFER_SIZE, 0);
                        if (byRecv == 0 || byRecv == -1) {
                            clientClose = true;
                            break;
                        }
                        file.write(bufferFile, byRecv);
                        fileDownloaded += byRecv;
                    } while (fileDownloaded < fileRequestedsize);
                    file.close();
                    std::cout << "File recived: "<< fileRequested<<std::endl;
                    clientClose = true;


                }
                else if (codeAvailable == 404) {
                    std::cout << "Can't open file or file not found!" << std::endl;
                }

            } while (!clientClose);
            key = 0;
            k++;

        }

        if (key == 2) {
            printf("Connection closing...\n");
            menu = false;
        }
    }
    // }
     // shutdown the connection since we're done
    iResult = shutdown(ClientSocket, SD_SEND);
    if (iResult == SOCKET_ERROR) {
        printf("shutdown failed with error: %d\n", WSAGetLastError());
        closesocket(ClientSocket);
        WSACleanup();
        return 1;
    }

    // cleanup
    closesocket(ClientSocket);
    WSACleanup();
    system("pause");
    return 0;
}