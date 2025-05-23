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
    SOCKET sock;
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
 * The returned SERVER instance must be freed by cli_core_logout.
 * @param ip IPv4 address string to the server.
 * @param port Port of the server.
 * @param callback Function to be called when new msg arrives.
 * @param wsa_error Pointer to an integer, to store WSA error code, or 0 if there are no WSA errors.
 * @return Pointer to a SERVER instance: success; NULL: failure.
 */
SERVER* cli_core_login(const char* ip, int port, int (*callback)(const char*, int len), int* wsa_error);

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
 * The SERVER instance will be freed and set to NULL.
 * @param server Pointer to a SERVER instance that has been initialized by cli_core_login.
 * @return 0: success, -1: internal error.
 */
int cli_core_logout(SERVER *server);

#endif //CLI_CORE_H
