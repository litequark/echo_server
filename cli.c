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
#include <windows.h>
#include "cli_core.h"

static const int svr_buf_len = 1024;

int print_msg(const char* msg, int len);

int receive(void *svr_sock);

int main(int argc, char *argv[])
{
    // Parse cmd args
    if (argc != 3)
    {
        fprintf(stderr, "Usage: %s <ip> <port>\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    const char* svr_ip = argv[1];
    const int svr_port = strtol(argv[2], NULL, 10);


    int i_ret = 0;

    // Initialize WSA
    i_ret = cli_core_init();
    if(i_ret != 0)
    {
        fprintf(stderr, "WSA initialization failed (%d)\n", i_ret);
        exit(EXIT_FAILURE);
    }

    // Connect to server
    SERVER* svr = cli_core_login(svr_ip, svr_port, print_msg,NULL);
    if (svr == NULL)
    {
        fprintf(stderr, "Failed to connect to server.\n");
        cli_core_cleanup();
        exit(EXIT_FAILURE);
    }

    printf("Connected.\n"
        "Type message and hit RETURN to send, \"***exit\" to disconnect.\n");


    char* buf = calloc(svr_buf_len, sizeof(char));
    do
    {
        fgets(buf, svr_buf_len, stdin);
        buf[strcspn(buf, "\n")] = '\0';
        i_ret = cli_core_send(svr, buf, (int)strlen(buf) * (int)sizeof(char));
        if (i_ret != 0)
        {
            fprintf(stderr, "Failed to send, err code:%d\n", i_ret);
        }
    }
    while (strcmp(buf, "***exit") != 0);
    cli_core_logout(svr);
    printf("Logout\n");

    cli_core_cleanup();
    free(buf);

    return 0;
}

int print_msg(const char* msg, int len)
{
    printf("[Someone] %s\n", msg);
    return 0;
}

int receive(void* svr_sock)
{
    SOCKET* sock = (SOCKET*)svr_sock;
    char *buf = calloc(svr_buf_len, sizeof(char));
    int i_ret = 0;
    while (1)
    {
        i_ret = recv(*sock, buf, svr_buf_len * (int)sizeof(char), 0);
        if (i_ret == SOCKET_ERROR)
        {
            fprintf(stderr, "recv failed with error: %d\n", WSAGetLastError());
            break;
        }
        if (i_ret == 0)
        {
            break;
        }
        // printf("[SOMEONE] %s\n", buf);
    }

    free(buf);
    return 0;
}
