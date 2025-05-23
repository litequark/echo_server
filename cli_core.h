//
// Created by zkm on 2025/5/20.

#ifndef CLI_CORE_H
#define CLI_CORE_H

#include <threads.h>
#include <stdlib.h>
#include <stdio.h>

#include <locale.h>
#include <winsock2.h>
#include <ws2tcpip.h>

typedef struct Server
{
    SOCKET sock;
} SERVER;

typedef struct CallbackFn_Params
{
    SOCKET *sock;
    int (*callback)(const char*, int len);
} CALLBACK_FN_PARAMS;

/**
 * Initializes locale and Windows socket.
 * MUST be called first.
 *
 * Exception: WSA network error.
 * @return 0: success; WSA error code: failure.
 */
int cli_core_init();

/**
 * Cleans up client.
 * MUST be called prior to exiting.
 *
 * Exception: WSA network error.
 * @return 0: success; WSA error code: failure.
 */
int cli_core_cleanup();

/**
 * Initialize a SERVER instance.
 * @param ip IPv4 address string to the server.
 * @param port Port of the server.
 * @param svr Pointer to a buffer that can contain a SERVER instance.
 * @param callback Function to be called when new msg arrives.
 * @return 0: success; WSA error code: WSA failure; -1: internal error.
 */
int cli_core_login(const char *ip, int port, SERVER* svr, int (*callback)(const char*, int len));

/**
 * Sends a message to the LAN.
 * @param server Pointer to a SERVER instance.
 * @param msg Message to be sent, in string.
 * @param len Length (bytes) of the message string.
 * @return 0: success; WSA error code: WSA failure; -1: internal error.
 */
int cli_core_send(SERVER *server, const char *msg, int len);

/**
 * Disconnect client from server.
 * @param server Pointer to an initialized SERVER instance.
 * @return 0: success, -1: internal error.
 */
int cli_core_logout(SERVER *server);

#endif //CLI_CORE_H
