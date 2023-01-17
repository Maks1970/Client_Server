#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include <fstream>
#include <iostream>


// Need to link with Ws2_32.lib, Mswsock.lib, and Advapi32.lib
#pragma comment (lib, "Ws2_32.lib")
#pragma comment (lib, "Mswsock.lib")
#pragma comment (lib, "AdvApi32.lib")


#define DEFAULT_BUFLEN 512
#define DEFAULT_PORT "27015"

int __cdecl main(int argc, char** argv)
{
    WSADATA wsaData;
    SOCKET ConnectSocket = INVALID_SOCKET;
    struct addrinfo* result = NULL,
        * ptr = NULL,
        hints;
    const char* sendbuf = "Hellow!";
    char msg[256] = "Hellow!";
    int sendmenu = 1;
    char recvbuf[DEFAULT_BUFLEN];
    int iResult;
    int recvbuflen = DEFAULT_BUFLEN;


    std::cout << "Kosheliuk Maksim \n";
    std::cout << "Student of group BKI-19 \n";
    std::cout << "Version 10 \n";
    // Initialize Winsock
    iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != 0) {
        printf("WSAStartup failed with error: %d\n", iResult);
        return 1;
    }

    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    // Resolve the server address and port
    iResult = getaddrinfo(argv[1], DEFAULT_PORT, &hints, &result);
    if (iResult != 0) {
        printf("getaddrinfo failed with error: %d\n", iResult);
        WSACleanup();
        return 1;
    }

    // Attempt to connect to an address until one succeeds
    for (ptr = result; ptr != NULL; ptr = ptr->ai_next) {

        // Create a SOCKET for connecting to server
        ConnectSocket = socket(ptr->ai_family, ptr->ai_socktype,
            ptr->ai_protocol);
        if (ConnectSocket == INVALID_SOCKET) {
            printf("socket failed with error: %ld\n", WSAGetLastError());
            WSACleanup();
            return 1;
        }

        // Connect to server.
        iResult = connect(ConnectSocket, ptr->ai_addr, (int)ptr->ai_addrlen);
        if (iResult == SOCKET_ERROR) {
            closesocket(ConnectSocket);
            ConnectSocket = INVALID_SOCKET;
            continue;
        }
        break;
    }

    freeaddrinfo(result);

    if (ConnectSocket == INVALID_SOCKET) {
        printf("Unable to connect to server!\n");
        WSACleanup();
        return 1;
    }
    bool menu = true;
    int key = 0;

    while (menu)
    {
       
        std::cout << " Vidpravit File ? (1) : " << std::endl;
        std::cout << " Disconect ? (2) : " << std::endl;
        std::cin >> key;
        int k = 0;
        char welcomeMsg[255];
        const int BUFFER_SIZE = 1024;
        char bufferFile[BUFFER_SIZE];
        char fileRequested[FILENAME_MAX];
        int byRecv;
        std::ofstream file;
        while (key == 1)
        {
            

            if (k == 0)send(ConnectSocket, (char*)&key, sizeof(key), 0);
            const char* welcomeMsg = "Welcome to file server.";
            bool clientClose = false;
            char fileRequested[FILENAME_MAX];
            const int fileAvailable = 200;
            const int fileNotfound = 404;
            const int BUFFER_SIZE = 1024;
            char bufferFile[BUFFER_SIZE];

            std::ifstream file;
            do {
                int fileDownloaded = 0;
                memset(fileRequested, 0, FILENAME_MAX);
                std::cout << "Enter file name: " << std::endl;
                std::cin >> fileRequested, FILENAME_MAX;
               byRecv = send(ConnectSocket, fileRequested, FILENAME_MAX, 0);
                if (byRecv == 0 || byRecv == -1) {
                    clientClose = true;
                    break;
                }

                file.open(fileRequested, std::ios::binary);
                if (file.is_open()) {
                    int bySendinfo = send(ConnectSocket, (char*)&fileAvailable, sizeof(int), 0);
                    if (bySendinfo == 0 || bySendinfo == -1) {
                        clientClose = true;
                    }
                    file.seekg(0, std::ios::end);
                    long fileSize = file.tellg();
                    bySendinfo = send(ConnectSocket, (char*)&fileSize, sizeof(long), 0);
                    if (bySendinfo == 0 || bySendinfo == -1) {
                        clientClose = true;
                    }
                    file.seekg(0, std::ios::beg);
                    do {
                        file.read(bufferFile, BUFFER_SIZE);
                        if (file.gcount() > 0)
                            bySendinfo = send(ConnectSocket, bufferFile, file.gcount(), 0);
                        if (bySendinfo == 0 || bySendinfo == -1) {
                            clientClose = true;
                            break;
                        }
                    } while (file.gcount() > 0);

                    file.close();
                    std::cout << "File sended!!!\n";
                    clientClose = true;
                    key = 0;
                }
                else {
                    int bySendCode = send(ConnectSocket, (char*)&fileNotfound, sizeof(int), 0);
                    if (bySendCode == 0 || bySendCode == -1) {
                        clientClose = true;
                    }
                }
            } while (!clientClose);
        }


        if (key == 2) {
            send(ConnectSocket, (char*)&key, sizeof(key), 0);
            printf("Connection closed\n");
            menu = false;
        }
    }
    iResult = shutdown(ConnectSocket, SD_RECEIVE);
    if (iResult == SOCKET_ERROR) {
        printf("shutdown failed with error: %d\n", WSAGetLastError());
        closesocket(ConnectSocket);
        WSACleanup();
        return 1;
    }
    // cleanup
    closesocket(ConnectSocket);
    WSACleanup();
    system("pause");
    return 0;
}