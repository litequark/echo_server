//
// Created by zkm on 2025/4/30.
//

#include <locale.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <threads.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <strsafe.h>
#define MAX_CLIENTS 1024

typedef struct client
{
    SOCKET sock;
    thrd_t thread;
} CLIENT;

const int svr_buf_len = 1024;
int next_cli_pos = 0;
CLIENT* clients = NULL;

int broadcast(void* cli);

int main(int argc, char* argv[])
{
    // Set encoding locale (not planned to use UTF-8 now)
    setlocale(LC_ALL, "zh-CN.gbk");

    // Check param count & assign param to vars
    if (argc != 3)
    {
        fprintf(stderr, "Usage: %s <ip> <port>\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    const char *svr_ip = argv[1];
    const int svr_port = (int)strtol(argv[2], NULL, 10);

    // Initialize CLIENT array
    clients = calloc(MAX_CLIENTS, sizeof(CLIENT));
    if (clients == NULL)
    {
        fprintf(stderr, "Failed to allocate memory for clients\n");
        exit(EXIT_FAILURE);
    }

    // Invalidate everyone in CLIENT array
    for (int i = 0; i < MAX_CLIENTS; i++)
    {
        clients[i].sock = INVALID_SOCKET;
    }

    // Network: Initialize WSA
    const WORD wsaVersionRequested = MAKEWORD(2, 2);
    WSADATA wsaData;
    int iRet = WSAStartup(wsaVersionRequested, &wsaData);
    if (iRet != 0)
    {
        fprintf(stderr, "WSAStartup failed with error: %d\n", iRet);
        free(clients);
        clients = NULL;
        exit(EXIT_FAILURE);
    }

    // Network: Create listening sock
    SOCKET sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sock == INVALID_SOCKET)
    {
        fprintf(stderr, "socket failed with error: %d\n", WSAGetLastError());
        WSACleanup();
        free(clients);
        clients = NULL;
        exit(EXIT_FAILURE);
    }
    struct sockaddr_in svr_addr;
    svr_addr.sin_family = AF_INET; // Internet IPv4
    svr_addr.sin_port = htons(svr_port);
    inet_pton(AF_INET, svr_ip, &svr_addr.sin_addr);

    // Network: Bind sock to local addresses
    iRet = bind(sock, (struct sockaddr*)&svr_addr, sizeof(svr_addr));
    if (iRet == SOCKET_ERROR)
    {
        fprintf(stderr, "bind failed with error: %d\n", WSAGetLastError());
        closesocket(sock);
        sock = INVALID_SOCKET;
        WSACleanup();
        free(clients);
        clients = NULL;
        exit(EXIT_FAILURE);
    }

    // Network: Start listening
    iRet = listen(sock, SOMAXCONN);
    if (iRet == SOCKET_ERROR)
    {
        fprintf(stderr, "listen failed with error: %d\n", WSAGetLastError());
        closesocket(sock);
        sock = INVALID_SOCKET;
        WSACleanup();
        free(clients);
        clients = NULL;
        exit(EXIT_FAILURE);
    }
    printf("Server listening on port %d\n", svr_port);

    while (1)
    {
        // Accept new client
        SOCKET tmp = accept(sock, NULL, NULL); // This WAITS for a client to connect
        if (tmp == INVALID_SOCKET)
        {
            fprintf(stderr, "accept failed with error: %d\n", WSAGetLastError());
            continue;
        }

        /* Find a vacant CLIENT position */
        // From current pos:
        int pos = 0;
        int found = 0;
        for (pos = next_cli_pos; pos < MAX_CLIENTS; pos++)
        {
            if (clients[pos].sock == INVALID_SOCKET)
            {
                next_cli_pos = pos;
                found = 1;
                break;
            }
        }
        // From beginning (in case that some clients has dropped):
        if (!found)
        {
            for (pos = 0; pos < next_cli_pos; pos++)
            {
                if (clients[pos].sock == INVALID_SOCKET)
                {
                    next_cli_pos = pos;
                    found = 1;
                    break;
                }
            }
        }
        if (!found)
        {
            fprintf(stderr, "Maximum number of clients reached.\n");
            const char max_error[] = {2};
            iRet = send(tmp, max_error, 1, 0);
            if (iRet == SOCKET_ERROR)
            {
                fprintf(stderr, "Failed to send OK signal, error: %d\n", WSAGetLastError());
                closesocket(tmp);
                tmp = INVALID_SOCKET;
                continue;
            }
            closesocket(tmp);
            tmp = INVALID_SOCKET;
            continue;
        }

        /* Client successfully accepted */
        // Send OK message (byte 00000001)
        const char ok[] = {1};
        iRet = send(tmp, ok, 1, 0);
        if (iRet == SOCKET_ERROR)
        {
            fprintf(stderr, "Failed to send OK signal, error: %d\n", WSAGetLastError());
            closesocket(tmp);
            tmp = INVALID_SOCKET;
            continue;
        }
        // Fill in object CLIENT
        clients[next_cli_pos].sock = tmp;
        iRet = thrd_create(&clients[next_cli_pos].thread, broadcast, (void*)&clients[next_cli_pos]);
        if (iRet != thrd_success)
        {
            fprintf(stderr, "could not create thread for client\n");
            closesocket(clients[next_cli_pos].sock);
            clients[next_cli_pos].sock = INVALID_SOCKET;
        }
        else
        {
            next_cli_pos++;
            printf("Client connected\n");
        }
    }

    closesocket(sock);
    sock = INVALID_SOCKET;
    WSACleanup();

    return 0;
}

int broadcast(void* cli)
{
    CLIENT* client = (CLIENT*)cli;
    char* buf = calloc(svr_buf_len, sizeof(char));
    int iRet = 0;
    do
    {
        iRet = recv(client->sock, buf, (svr_buf_len) * (int)sizeof(char), 0);
        buf[svr_buf_len - 1] = '\0';
        if (iRet == SOCKET_ERROR)
        {
            fprintf(stderr, "recv failed with error: %d\n", WSAGetLastError());

            closesocket(client->sock);
            client->sock = INVALID_SOCKET;
            free(buf);
            buf = NULL;
            return 1;
        }
        else if (iRet == 0)
        {
            fprintf(stdout, "Connection closed gracefully.\n");
            closesocket(client->sock);
            client->sock = INVALID_SOCKET;
            break;
        }

        printf("[CLIENT] %s\n", buf);

        for (int i = 0; i < MAX_CLIENTS; i++)
        {
            if (clients[i].sock == client->sock || clients[i].sock == INVALID_SOCKET)
            {
                continue;
            }
            int sendRet = send(clients[i].sock, buf, (svr_buf_len) * (int)sizeof(char), 0);
            if (sendRet == SOCKET_ERROR)
            {
                fprintf(stderr, "Broadcast to a sock failed with error: %d\n", WSAGetLastError());

                closesocket(clients[i].sock);
                clients[i].sock = INVALID_SOCKET;
            }
        }
        printf("[BROADCAST] %s\n", buf);

        memset(buf, '\0', svr_buf_len * sizeof(char));
    }
    while (iRet > 0);

    closesocket(client->sock);
    client->sock = INVALID_SOCKET;
    free(buf);
    buf = NULL;
    return 0;
}
