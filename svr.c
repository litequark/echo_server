//
// Created by zkm on 2025/4/30.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>

const char* svr_ip = "0.0.0.0";
int svr_port = 11451;
const int svr_buf_len = 1024;

int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        fprintf(stderr, "Usage: %s <ip> <port>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    svr_ip = argv[1];
    svr_port = strtol(argv[2], NULL, 10);

    const WORD wsaVersionRequested = MAKEWORD(2, 2);
    WSADATA wsaData;
    int iRet = WSAStartup(wsaVersionRequested, &wsaData);
    if (iRet != 0)
    {
        fprintf(stderr, "WSAStartup failed with error: %d\n", iRet);
        exit(EXIT_FAILURE);
    }

    SOCKET sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sock == INVALID_SOCKET)
    {
        fprintf(stderr, "socket failed with error: %d\n", WSAGetLastError());
        WSACleanup();
        exit(EXIT_FAILURE);
    }
    struct sockaddr_in svr_addr;
    svr_addr.sin_family = AF_INET; // Internet IPv4
    svr_addr.sin_port = htons(svr_port);
    svr_addr.sin_addr.s_addr = inet_addr(svr_ip);

    iRet = bind(sock, (struct sockaddr*)&svr_addr, sizeof(svr_addr));
    if (iRet == SOCKET_ERROR)
    {
        fprintf(stderr, "bind failed with error: %d\n", WSAGetLastError());
        closesocket(sock);
        WSACleanup();
        exit(EXIT_FAILURE);
    }
    iRet = listen(sock, SOMAXCONN);
    if (iRet == SOCKET_ERROR)
    {
        fprintf(stderr, "listen failed with error: %d\n", WSAGetLastError());
        closesocket(sock);
        WSACleanup();
        exit(EXIT_FAILURE);
    }

    printf("Server listening on port %d\n", svr_port);

    SOCKET cli_sock = accept(sock, NULL, NULL);
    if (cli_sock == INVALID_SOCKET)
    {
        fprintf(stderr, "accept failed with error: %d\n", WSAGetLastError());
        closesocket(sock);
        WSACleanup();
        exit(EXIT_FAILURE);
    }

    printf("Client connected\n");


    char* buf = calloc(svr_buf_len, sizeof(char));
    do
    {
        iRet = recv(cli_sock, buf, (svr_buf_len) * sizeof(char), 0);
        buf[svr_buf_len - 1] = '\0';
        if (iRet == SOCKET_ERROR)
        {
            fprintf(stderr, "recv failed with error: %d\n", WSAGetLastError());

                closesocket(cli_sock);
                closesocket(sock);
                free(buf);
                WSACleanup();
                exit(EXIT_FAILURE);

        }
        else if (iRet == 0)
        {
            fprintf(stdout, "Connection closed gracefully.\n");
            break;
        }

        printf("[CLIENT] %s\n", buf);
        iRet = send(cli_sock, buf, (svr_buf_len) * sizeof(char), 0);
        if (iRet == SOCKET_ERROR)
        {
            fprintf(stderr, "send failed with error: %d\n", WSAGetLastError());

                closesocket(cli_sock);
                closesocket(sock);
                free(buf);
                WSACleanup();
                exit(EXIT_FAILURE);

        }
        printf("[SERVER] %s\n", buf);

        memset(buf, '\0', svr_buf_len * sizeof(char));
    }
    while (iRet > 0);

    free(buf);

    closesocket(cli_sock);
    closesocket(sock);
    WSACleanup();

    return 0;
}
