//
// Created by zkm on 2025/4/30.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>

const char* svr_ip = "127.0.0.1";
const int svr_port = 11451;
const int svr_buf_len = 1024;

int main()
{
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

    iRet = connect(sock, (struct sockaddr*)&svr_addr, sizeof(svr_addr));
    if (iRet == SOCKET_ERROR)
    {
        fprintf(stderr, "connect failed with error: %d\n", WSAGetLastError());
        closesocket(sock);
        WSACleanup();
        exit(EXIT_FAILURE);
    }

    printf("Connected.\n"
        "Type message and hit RETURN to send, \"***exit\" to disconnect.\n");

    char* buf = calloc(svr_buf_len, sizeof(char));
    do
    {
        printf("[CLIENT] ");
        fgets(buf, svr_buf_len, stdin);
        buf[strcspn(buf, "\n")] = '\0';
        iRet = send(sock, buf, svr_buf_len * sizeof(char), 0);
        if (iRet == SOCKET_ERROR)
        {
            fprintf(stderr, "send failed with error: %d\n", WSAGetLastError());
            closesocket(sock);
            free(buf);
            WSACleanup();
            exit(EXIT_FAILURE);
        }

        iRet = recv(sock, buf, svr_buf_len * sizeof(char), 0);
        buf[svr_buf_len - 1] = '\0';
        if (iRet == SOCKET_ERROR)
        {
            fprintf(stderr, "recv failed with error: %d\n", WSAGetLastError());
            closesocket(sock);
            free(buf);
            WSACleanup();
            exit(EXIT_FAILURE);
        }
        printf("[SERVER] %s\n", buf);
    }
    while (strcmp(buf, "***exit") != 0);
    shutdown(sock, SD_SEND);
    iRet = 1;
    do
    {
        iRet = recv(sock, buf, svr_buf_len * sizeof(char), 0);
    }
    while (iRet != 0);

    printf("Connection closed gracefully by server.\n");
    closesocket(sock);
    WSACleanup();
    free(buf);

    return 0;
}
