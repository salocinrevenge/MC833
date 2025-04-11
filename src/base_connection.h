#ifndef BASE_CONNECTION_H
#define BASE_CONNECTION_H

#include <netinet/in.h>

#define PORT "3490"      // the port users will be connecting to
#define MAXDATASIZE 1000 // max number of bytes we can send at once

/**
 * Structure used to return metadata about a received message.
 *
 * Members:
 * - len: Length of the message received (not including null terminator).
 * - continuando: Flag indicating whether more messages are expected (1) or not (0).
 */
typedef struct retorno_recv
{
    uint32_t len;
    uint8_t continuando;
} retorno_recv;

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
 * Sends a message to the specified socket with a custom protocol:
 * - 4-byte header containing the message length (including null terminator),
 * - 1-byte continuation flag ('1' or '0'),
 * - message body (null-terminated string).
 *
 * @param fd Socket file descriptor.
 * @param msg Null-terminated string to send.
 * @param continuar If 1, indicates more messages will follow; if 0, this is the final message.
 * @return Number of bytes sent from the message body (excluding header), or 0 on error.
 */
size_t send_message(int fd, const char *buf, int continuar);

/**
 * Receives a message from the specified socket using the custom protocol.
 * Expects a 4-byte length header, a 1-byte continuation flag, and a message body.
 * Allocates memory for the received message and stores the pointer in `buf`.
 *
 * @param fd Socket file descriptor.
 * @param buf Pointer to a char pointer where the dynamically allocated message will be stored.
 * @return A `retorno_recv` struct containing:
 *         - the length of the message (excluding null terminator),
 *         - whether more messages are expected (continuando).
 *         Returns {0, 0} on failure.
 */
retorno_recv recv_message(int fd, char **buf);

#endif // BASE_CONNECTION_H
