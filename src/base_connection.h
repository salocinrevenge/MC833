#ifndef BASE_CONNECTION_H
#define BASE_CONNECTION_H

#include <netinet/in.h>

#define PORT "3490"         // the port users will be connecting to
#define MAXDATASIZE 1000    // max number of bytes we can send at once

// Get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa);

// Signal handler for SIGCHLD to reap zombie processes:
void sigchld_handler(int s);

// Send a message through the given socket file descriptor:
size_t send_message(int fd, const char *buf);

// Receives a dynamically-sized, null-terminated message from 'fd'.
// Allocates memory for the message and stores it in '*buf'.
// Returns the number of bytes received (excluding the null terminator), or 0 on error.
size_t recv_message(int fd, char **buf);

#endif // BASE_CONNECTION_H
