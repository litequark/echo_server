//
// Created by zkm on 2025/4/30.
//

#include <locale.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <tchar.h>
#include <strsafe.h>
#include <process.h>
#include <windows.h>
#define MAX_CLIENTS 1000

const char* svr_ip = "0.0.0.0";
int svr_port = 11451;
const int svr_buf_len = 1024;
int cli_count = 0;
SOCKET cli_socks[MAX_CLIENTS] = {INVALID_SOCKET};
HANDLE cli_handles[MAX_CLIENTS] = {0};
unsigned cli_thread_ids[MAX_CLIENTS] = {0};

unsigned __stdcall echo(void *cli);

int main(int argc, char* argv[])
{
    memset(&cli_socks, INVALID_SOCKET, sizeof(cli_socks));
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

    iRet = bind(sock, (struct sockaddr*)&svr_addr, sizeof(svr_addr));
    if (iRet == SOCKET_ERROR)
    {
        fprintf(stderr, "bind failed with error: %d\n", WSAGetLastError());
        closesocket(sock);
        sock = INVALID_SOCKET;
        WSACleanup();
        exit(EXIT_FAILURE);
    }
    iRet = listen(sock, SOMAXCONN);
    if (iRet == SOCKET_ERROR)
    {
        fprintf(stderr, "listen failed with error: %d\n", WSAGetLastError());
        closesocket(sock);
        sock = INVALID_SOCKET;
        WSACleanup();
        exit(EXIT_FAILURE);
    }

    printf("Server listening on port %d\n", svr_port);

    int cli_count = 0;
    while (1)
    {
        cli_socks[cli_count] = accept(sock, NULL, NULL);

        if (cli_socks[cli_count] == INVALID_SOCKET)
        {
            fprintf(stderr, "accept failed with error: %d\n", WSAGetLastError());
        }
        else
        {
            // cli socket success, pass it to a new thread
            cli_handles[cli_count] = (HANDLE)_beginthreadex(
                NULL,
                0,
                echo,
                (void*)&(cli_socks[cli_count]),
                0,
                &cli_thread_ids[cli_count]);
            if (cli_handles[cli_count] == NULL)
            {
                fprintf(stderr, "could not create thread for client\n");
                closesocket(cli_socks[cli_count]);
                cli_socks[cli_count] = INVALID_SOCKET;
            }
            else
            {
                printf("Client connected\n");
                cli_count++;
            }
        }
    }


    closesocket(sock);
    sock = INVALID_SOCKET;
    WSACleanup();

    return 0;
}

unsigned echo(void* cli)
{
    SOCKET* sock = (SOCKET*)cli;
    char* buf = calloc(svr_buf_len, sizeof(char));
    int iRet = 0;
    do
    {
        iRet = recv(*sock, buf, (svr_buf_len) * (int)sizeof(char), 0);
        buf[svr_buf_len - 1] = '\0';
        if (iRet == SOCKET_ERROR)
        {
            fprintf(stderr, "recv failed with error: %d\n", WSAGetLastError());

            closesocket(*sock);
            *sock = INVALID_SOCKET;
            free(buf);
            buf = NULL;
            _endthreadex(1);
            return 1;
        }
        else if (iRet == 0)
        {
            fprintf(stdout, "Connection closed gracefully.\n");
            closesocket(*sock);
            *sock = INVALID_SOCKET;
            break;
        }

        printf("[CLIENT] %s\n", buf);

        for (int i = 0; i < cli_count; i++)
        {
            if (cli_socks[i] == *sock || cli_socks[i] == INVALID_SOCKET)
            {
                continue;
            }
            iRet = send(cli_socks[i], buf, (svr_buf_len) * (int)sizeof(char), 0);
            if (iRet == SOCKET_ERROR)
            {
                fprintf(stderr, "send failed with error: %d\n", WSAGetLastError());

                closesocket(*sock);
                *sock = INVALID_SOCKET;
                free(buf);
                buf = NULL;
                _endthreadex(1);
                return 1;
            }
        }
        printf("[SOMEONE] %s\n", buf);

        memset(buf, '\0', svr_buf_len * sizeof(char));
    }
    while (iRet > 0);
    closesocket(*sock);
    *sock = INVALID_SOCKET;
    free(buf);
    buf = NULL;
    _endthreadex(0);
    return 0;
}
