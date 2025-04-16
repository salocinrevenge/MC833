#ifndef BASE_CONNECTION_H
#define BASE_CONNECTION_H

#include <stdbool.h>
#include <netinet/in.h>

#define PORT "3490"      // the port users will be connecting to
#define MAXDATASIZE 1000 // max number of bytes we can send at once

/**
 * Structure used to represent the message header exchanged between client and server.
 *
 * Members:
 * - len: Length of the message (including null terminator when sending, excluding when received).
 * - continuando: Boolean flag indicating whether additional messages are expected (1) or not (0).
 */
typedef struct header
{
    uint32_t len;
    bool continuando;
} header;

/**
 * Extracts the IP address (IPv4 or IPv6) from a generic sockaddr structure.
 *
 * @param sa Pointer to a struct sockaddr (may be IPv4 or IPv6).
 * @return Pointer to the IP address portion of the sockaddr.
 */
void *get_in_addr(struct sockaddr *sa);

/**
 * Signal handler for SIGCHLD to reap zombie child processes.
 * Uses waitpid in a non-blocking loop.
 *
 * @param s Signal number (unused).
 */
void sigchld_handler(int s);

/**
 * Sends a message to the given socket using a custom protocol:
 *
 * - full header (message length + continuation flag),
 *
 * - message body (null-terminated string).
 *
 * @param fd Socket file descriptor to send the message through.
 * @param msg Null-terminated message string to send.
 * @param continuar Boolean indicating whether more messages will follow (1) or not (0).
 * @return Number of bytes sent from the message body (excluding header), or 0 on error.
 */
size_t send_message(int fd, const char *buf, bool continuar);

/**
 * Receives a message from the socket according to the custom protocol:
 *
 * - Receives a full header (message length + continuation flag),
 *
 * - Allocates a buffer for the message body and reads it into memory.
 *
 * @param fd Socket file descriptor to receive the message from.
 * @param buf Address of a pointer where the allocated message buffer will be stored.
 * @return A `header` struct containing the actual length of the received message (excluding null terminator)
 *         and the continuation flag. If an error occurs, `len` will be -1.
 */
header recv_message(int fd, char **buf);

#endif // BASE_CONNECTION_H
