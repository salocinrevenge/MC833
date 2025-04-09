#include "base_connection.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>

#include <arpa/inet.h>

void falar_com_server(int fd) {
    char *received;
    char to_send[MAXDATASIZE];

    while(1) {
        int lendo;

        do {
            lendo = 0;

            retorno_recv retorno = recv_message(fd, &received);
            if (retorno.len == 0) {
                exit(1);
            }

            // printf("client: received '%s'\n", received);

            if(retorno.continuando == 1) {
                lendo = 1;
            }

            printf(">> %s\n",received);

            // Check exit condition
            if (strcmp(received, "Recebido '8', encerrando acesso.") == 0) {
                printf("Encerrada conexão com o servidor.\n");
                free(received);
                return;
            }

            free(received);
        } while(lendo);

        // Prompt user for input
        printf("> ");
        fgets(to_send, MAXDATASIZE, stdin);
        to_send[strcspn(to_send, "\n")] = '\0';

        send_message(fd, to_send, 0);
    }
}

int main(int argc, char *argv[])
{
    int sockfd, numbytes;  
    struct addrinfo hints, *servinfo, *p;
    int rv;
    char s[INET6_ADDRSTRLEN];

    if (argc != 2) {
        fprintf(stderr,"usage: client hostname\n");
        exit(1);
    }

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    if ((rv = getaddrinfo(argv[1], PORT, &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }

    // loop through all the results and connect to the first we can
    for(p = servinfo; p != NULL; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype,
                p->ai_protocol)) == -1) {
            perror("client: socket");
            continue;
        }

        if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(sockfd);
            perror("client: connect");
            continue;
        }

        break;
    }

    if (p == NULL) {
        fprintf(stderr, "client: failed to connect\n");
        return 2;
    }

    inet_ntop(p->ai_family, get_in_addr((struct sockaddr *)p->ai_addr),
            s, sizeof s);
    printf("client: connecting to %s\n", s);

    freeaddrinfo(servinfo); // all done with this structure

    falar_com_server(sockfd);

    close(sockfd);

    return 0;
}