#include "base_connection.h"

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/wait.h>

void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET)
        return &(((struct sockaddr_in *)sa)->sin_addr);

    return &(((struct sockaddr_in6 *)sa)->sin6_addr);
}

void sigchld_handler(int s)
{
    // waitpid() might overwrite errno, so we save and restore it
    int saved_errno = errno;

    while (waitpid(-1, NULL, WNOHANG) > 0)
        ;

    errno = saved_errno;
}

size_t send_message(int fd, const char *msg, bool continuar)
{
    uint32_t len = strlen(msg) + 1; // include null terminator
    uint32_t len_net = htonl(len);  // convert to network byte order

    header header;
    header.len = len_net;
    header.continuando = continuar;

    // Send the header
    if (send(fd, &header, sizeof(header), 0) != sizeof(header))
    {
        perror("send message header");
        return 0;
    }

    // Send the message body
    size_t total_sent = 0;
    while (total_sent < len) // handle partial sends
    {
        size_t chunk_size = (len - total_sent > MAXDATASIZE) ? MAXDATASIZE : (len - total_sent);
        ssize_t bytes_sent = send(fd, msg + total_sent, chunk_size, 0);
        if (bytes_sent == -1)
        {
            perror("send message body");
            return 0;
        }
        total_sent += bytes_sent;
    }

    return total_sent;
}

header recv_message(int fd, char **buf)
{
    header header;
    header.len = 0;
    header.continuando = 0;

    if (!buf)
        return header;

    // Receive the header
    ssize_t bytes_received = recv(fd, &header, sizeof(header), MSG_WAITALL);
    if (bytes_received != sizeof(header))
    {
        perror("recv header");
        header.len = -1; // indicate error
        return header;
    }
    header.len = ntohl(header.len);

    if (header.len == 0)
    {
        fprintf(stderr, "Warning: received message with length 0\n");
        return header;
    }

    // Allocate a buffer for the message
    *buf = malloc(header.len);
    if (!*buf)
    {
        perror("malloc");
        // Drain the message from the socket to maintain protocol sync
        char drain_buf[1024];
        size_t remaining = header.len;
        while (remaining > 0)
        {
            size_t to_read = (remaining > sizeof(drain_buf)) ? sizeof(drain_buf) : remaining;
            ssize_t drained = recv(fd, drain_buf, to_read, 0);
            if (drained <= 0)
                break;
            remaining -= drained;
        }
        header.len = -1;
        return header;
    }

    // Receive the message body
    size_t total_received = 0;
    while (total_received < header.len)
    {
        size_t chunk_size = (header.len - total_received > MAXDATASIZE) ? MAXDATASIZE : (header.len - total_received);
        bytes_received = recv(fd, *buf + total_received, chunk_size, 0);
        if (bytes_received <= 0)
        {
            perror("recv message body");
            free(*buf);
            *buf = NULL;
            header.len = -1;
            return header;
        }
        total_received += bytes_received;
    }

    (*buf)[header.len - 1] = '\0';
    header.len--;
    return header;
}