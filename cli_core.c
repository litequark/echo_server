//
// Created by zkm on 2025/5/20.
//

#include "cli_core.h"

#include <assert.h>

const int svr_buf_len = 1024;


static int receive(void* args);


int cli_core_init()
{
    setlocale(LC_ALL, "zh-CN.gbk");

    const WORD wsaVersionRequested = MAKEWORD(2, 2);
    WSADATA wsaData;
    int iRet = WSAStartup(wsaVersionRequested, &wsaData);
    if (iRet != 0)
    {
        return WSAGetLastError();
    }
    else
    {
        return 0;
    }
}

int cli_core_cleanup()
{
    WSACleanup();
    return 0;
}

int cli_core_login(const char* ip, int port, SERVER* svr, int (*callback)(const char*, int len))
{
    svr = calloc(1, sizeof(SERVER));
    SOCKET sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sock == INVALID_SOCKET)
    {
        int err = WSAGetLastError();
        cli_core_cleanup();
        svr->sock = INVALID_SOCKET;
        return err;
    }
    struct sockaddr_in svr_addr;
    svr_addr.sin_family = AF_INET; // Internet IPv4
    svr_addr.sin_port = htons(port);
    inet_pton(AF_INET, ip, &svr_addr.sin_addr.s_addr);

    int iRet = connect(sock, (struct sockaddr*)&svr_addr, sizeof(svr_addr));
    if (iRet == SOCKET_ERROR)
    {
        int err = WSAGetLastError();
        cli_core_cleanup();
        svr->sock = INVALID_SOCKET;
        return err;
    }

    // Initial connection established, waiting for server's response
    char ok_buf[1] = {0};
    iRet = recv(sock, ok_buf, sizeof(ok_buf), 0);
    if (iRet == SOCKET_ERROR)
    {
        // Failed to receive OK signal
        closesocket(sock);
        sock = INVALID_SOCKET;
        int err = WSAGetLastError();
        cli_core_cleanup();
        svr->sock = INVALID_SOCKET;
        return err;
    }
    if (ok_buf[0] != 1)
    {
        // Server responded with error signal
        closesocket(sock);
        sock = INVALID_SOCKET;
        int err = -1;
        cli_core_cleanup();
        svr->sock = INVALID_SOCKET;
        return err;
    }
    // success, write to SERVER*
    svr->sock = sock;

    // register callback function
    CALLBACK_FN_PARAMS args = {&svr->sock, receive};
    thrd_t receive_thread;
    iRet = thrd_create(&receive_thread, receive, (void *)&args);
    if (iRet != thrd_success)
    {
        // fprintf(stderr, "Failed to create receive thread.\n");
        closesocket(sock);
        sock = INVALID_SOCKET;
        cli_core_cleanup();
        return -1;
    }

    return 0;
}

int cli_core_send(SERVER* server, const char* msg, int len)
{
    int iRet = send(server->sock, msg, (int)strlen(msg) * (int)sizeof(char), 0);
    if (iRet == SOCKET_ERROR)
    {
        // fprintf(stderr, "send failed with error: %d\n", WSAGetLastError());
        int err = WSAGetLastError();
        closesocket(server->sock);
        server->sock = INVALID_SOCKET;
        cli_core_cleanup();
        return err;
    }
    else
    {
        return 0;
    }
}

int cli_core_logout(SERVER* server)
{
    shutdown(server->sock, SD_BOTH);
    closesocket(server->sock);
    server->sock = INVALID_SOCKET;
    free(server);
    return 0;
}

static int receive(void* args)
{
    assert(args != NULL);
    CALLBACK_FN_PARAMS* params = (CALLBACK_FN_PARAMS*)args;
    SOCKET* sock = (SOCKET*)(params->sock);
    int (*callback)(const char*, int) = params->callback;

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
        callback(buf, (int)strlen(buf));
        printf("[SOMEONE] %s\n", buf);
    }

    *sock = INVALID_SOCKET;
    free(buf);
    return 0;
}
