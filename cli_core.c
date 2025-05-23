//
// Created by zkm on 2025/5/20.
//

#include "cli_core.h"
#include <assert.h>

static const int svr_buf_len = 1024;

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

SERVER* cli_core_login(const char* ip, int port, int (*callback)(const char*, int len), int* wsa_error)
{
    if (ip == NULL || callback == NULL || wsa_error == NULL)
    {
        if (wsa_error != NULL)
        {
            *wsa_error = 0;
        }
        return NULL;
    }
    SERVER* svr = calloc(1, sizeof(SERVER));
    if (svr == NULL)
    {
        *wsa_error = 0;
        return NULL;
    }
    SOCKET sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sock == INVALID_SOCKET)
    {
        *wsa_error = WSAGetLastError();
        svr->sock = INVALID_SOCKET;
        free(svr);
        svr = NULL;

        return NULL;
    }
    struct sockaddr_in svr_addr;
    svr_addr.sin_family = AF_INET; // Internet IPv4
    svr_addr.sin_port = htons(port);
    inet_pton(AF_INET, ip, &svr_addr.sin_addr.s_addr);

    int i_ret = connect(sock, (struct sockaddr*)&svr_addr, sizeof(svr_addr));
    if (i_ret == SOCKET_ERROR)
    {
        *wsa_error = WSAGetLastError();
        svr->sock = INVALID_SOCKET;
        free(svr);
        svr = NULL;
        return NULL;
    }

    // Initial connection established, waiting for server's response
    char ok_buf[1] = {0};
    i_ret = recv(sock, ok_buf, sizeof(ok_buf), 0);
    if (i_ret == SOCKET_ERROR)
    {
        // Failed to receive OK signal
        closesocket(sock);
        sock = INVALID_SOCKET;
        *wsa_error = WSAGetLastError();
        svr->sock = INVALID_SOCKET;
        free(svr);
        svr = NULL;
        return NULL;
    }
    if (ok_buf[0] != 1)
    {
        // Server responded with error signal
        closesocket(sock);
        sock = INVALID_SOCKET;
        *wsa_error = 0;
        svr->sock = INVALID_SOCKET;
        free(svr);
        svr = NULL;
        return NULL;
    }
    // success, write to SERVER*
    svr->sock = sock;

    // register callback function
    // 线程回调函数的参数不要传局部变量的指针，否则局部变量自动析构后指针就无效了，而此时线程仍在运行，会造成access violation。
    CALLBACK_FN_PARAMS* p_args = calloc(1, sizeof(CALLBACK_FN_PARAMS));
    CALLBACK_FN_PARAMS args = {svr->sock, callback};
    *p_args = args;
    thrd_t receive_thread;
    i_ret = thrd_create(&receive_thread, receive, (void *)p_args);
    if (i_ret != thrd_success)
    {
        // fprintf(stderr, "Failed to create receive thread.\n");
        *wsa_error = 0;

        closesocket(sock);
        sock = INVALID_SOCKET;
        svr->sock = INVALID_SOCKET;
        free(svr);
        svr = NULL;
        free(p_args);
        p_args = NULL;
        return NULL;
    }

    return svr;
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
        return err;
    }
    else
    {
        return 0;
    }
}

int cli_core_logout(SERVER* server)
{
    char buf[1024] = {0};
    int i_ret = 0;
    // Shutdown sending side
    shutdown(server->sock, SD_SEND);
    // Complete remaining receiving task
    do
    {
        i_ret = recv(server->sock, buf, svr_buf_len * (int)sizeof(char), 0);
    }
    while (i_ret != 0);


    closesocket(server->sock);
    server->sock = INVALID_SOCKET;
    free(server);
    server = NULL;
    return 0;
}

static int receive(void* args)
{
    assert(args != NULL);
    CALLBACK_FN_PARAMS* params = (CALLBACK_FN_PARAMS*)args;
    if (params == NULL || params->sock == INVALID_SOCKET || params->callback == NULL)
    {
        return -1;
    }
    SOCKET sock = (SOCKET)(params->sock);
    int (*callback)(const char*, int) = params->callback;

    char *buf = calloc(svr_buf_len, sizeof(char));
    int i_ret = 0;
    while (1)
    {
        i_ret = recv(sock, buf, svr_buf_len * (int)sizeof(char), 0);
        if (i_ret == SOCKET_ERROR)
        {
            fprintf(stderr, "recv failed with error: %d\n", WSAGetLastError());
            break;
        }
        if (i_ret == 0)
        {
            break;
        }
        callback(buf, (int)strlen(buf));
        printf("[SOMEONE] %s\n", buf);
    }

    sock = INVALID_SOCKET;
    free(buf);
    free(args);
    args = NULL;
    return 0;
}
