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

size_t send_message(int fd, const char *msg)
{
    uint32_t len = strlen(msg) + 1; // include null terminator
    uint32_t len_net = htonl(len);  // convert to network byte order

    // Send the 4-byte header
    if (send(fd, &len_net, sizeof(len_net), 0) != sizeof(len_net))
    {
        perror("send message header");
        return 0;
    }

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

size_t recv_message(int fd, char **buf)
{
    if (!buf)
        return 0;

    uint32_t len_net;
    ssize_t bytes_received = recv(fd, &len_net, sizeof(len_net), MSG_WAITALL);
    if (bytes_received != sizeof(len_net))
    {
        perror("recv length header");
        return 0;
    }

    uint32_t len = ntohl(len_net);

    if (len == 0)
    {
        fprintf(stderr, "Warning: received message with length 0\n");
        return 0;
    }

    *buf = malloc(len);
    if (!*buf)
    {
        perror("malloc");
        // Drain the message from the socket to maintain protocol sync
        char drain_buf[1024];
        size_t remaining = len;
        while (remaining > 0)
        {
            size_t to_read = (remaining > sizeof(drain_buf)) ? sizeof(drain_buf) : remaining;
            ssize_t drained = recv(fd, drain_buf, to_read, 0);
            if (drained <= 0)
                break;
            remaining -= drained;
        }
        return 0;
    }

    size_t total_received = 0;
    while (total_received < len)
    {
        size_t chunk_size = (len - total_received > MAXDATASIZE) ? MAXDATASIZE : (len - total_received);
        bytes_received = recv(fd, *buf + total_received, chunk_size, 0);
        if (bytes_received <= 0)
        {
            perror("recv message body");
            free(*buf);
            *buf = NULL;
            return 0;
        }
        total_received += bytes_received;
    }

    (*buf)[len - 1] = '\0';
    return len - 1;
}
