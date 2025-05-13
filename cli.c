//
// Created by zkm on 2025/4/30.
//

#include <locale.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <process.h>
#include <windows.h>

const char* svr_ip = "127.0.0.1";
int svr_port = 11451;
const int svr_buf_len = 1024;

unsigned __stdcall receive(void *svr_sock);

int main(int argc, char *argv[])
{
    setlocale(LC_ALL, "zh-CN.gbk");
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

    iRet = connect(sock, (struct sockaddr*)&svr_addr, sizeof(svr_addr));
    if (iRet == SOCKET_ERROR)
    {
        fprintf(stderr, "connect failed with error: %d\n", WSAGetLastError());
        closesocket(sock);
        WSACleanup();
        exit(EXIT_FAILURE);
    }

    printf("Initial connection established, waiting for server's response ...\n");

    char ok_buf[1] = {0};
    iRet = recv(sock, ok_buf, sizeof(ok_buf), 0);
    if (iRet == SOCKET_ERROR)
    {
        fprintf(stderr, "Failed to receive OK signal, error: %d\n", WSAGetLastError());
        closesocket(sock);
        WSACleanup();
        exit(EXIT_FAILURE);
    }
    if (ok_buf[0] != 1)
    {
        fprintf(stderr, "Server responded with error signal.\n");
        closesocket(sock);
        WSACleanup();
        exit(EXIT_FAILURE);
    }

    printf("Connected.\n"
        "Type message and hit RETURN to send, \"***exit\" to disconnect.\n");

    HANDLE hThread = (HANDLE)_beginthreadex(
        NULL,
        0,
        receive,
        &sock,
        0,
        NULL);

    if (hThread == NULL)
    {
        fprintf(stderr, "create thread failed with error: %d\n", GetLastError());
        closesocket(sock);
        sock = INVALID_SOCKET;
        WSACleanup();
        exit(EXIT_FAILURE);
    }

    char* buf = calloc(svr_buf_len, sizeof(char));
    do
    {
        fgets(buf, svr_buf_len, stdin);
        buf[strcspn(buf, "\n")] = '\0';
        iRet = send(sock, buf, (int)strlen(buf) * (int)sizeof(char), 0);
        if (iRet == SOCKET_ERROR)
        {
            fprintf(stderr, "send failed with error: %d\n", WSAGetLastError());
            closesocket(sock);
            sock = INVALID_SOCKET;
            free(buf);
            WSACleanup();
            exit(EXIT_FAILURE);
        }


    }
    while (strcmp(buf, "***exit") != 0);
    shutdown(sock, SD_SEND);
    do
    {
        iRet = recv(sock, buf, svr_buf_len * (int)sizeof(char), 0);
    }
    while (iRet != 0);

    printf("Connection closed gracefully by server.\n");
    closesocket(sock);
    sock = INVALID_SOCKET;
    WSACleanup();
    free(buf);

    return 0;
}

unsigned __stdcall receive(void* svr_sock)
{
    SOCKET* sock = (SOCKET*)svr_sock;
    char *buf = calloc(svr_buf_len, sizeof(char));
    int iRet = 0;
    while (1)
    {
        iRet = recv(*sock, buf, svr_buf_len * (int)sizeof(char), 0);
        if (iRet == SOCKET_ERROR)
        {
            fprintf(stderr, "recv failed with error: %d\n", WSAGetLastError());
            break;
        }
        if (iRet == 0)
        {
            break;
        }
        printf("[SOMEONE] %s\n", buf);
    }

    free(buf);
    _endthreadex(0);
    return 0;
}
