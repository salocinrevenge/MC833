/*
** server.c -- a stream socket server demo
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>

#define PORT "3490"  // the port users will be connecting to

#define BACKLOG 10   // how many pending connections queue will hold

#define MAXDATASIZE 1000 // max number of bytes we can get at once 

void sigchld_handler(int s)
{
    // waitpid() might overwrite errno, so we save and restore it:
    int saved_errno = errno;

    while(waitpid(-1, NULL, WNOHANG) > 0);

    errno = saved_errno;
}


// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

void option1(int new_fd)
{
    int valid = 0;
    while(valid){
        const char *msg = "Você escolheu a opção 1: Cadastrar um novo filme.\n\
        Por favor, digite o nome do filme que deseja cadastrar.\n";
        if (send(new_fd, msg, strlen(msg), 0) == -1)
            perror("send");

        int numbytes = 0;
        char mensagem_recebida[MAXDATASIZE];
        if ((numbytes = recv(new_fd, mensagem_recebida, MAXDATASIZE, 0)) == -1)
            perror("recv");
        


    }
    

    
}

int atender(int new_fd)
{
    const char *msg = "Olá, cliente! Você se conectou ao servidor com sucesso!\n\
    Você pode realizar as seguintes operações:\n\
    (1) Cadastrar um novo filme\n\
    (2) Adicionar um novo gênero a um filme\n\
    (3) Remover um filme pelo identificador\n\
    (4) Listar todos os títulos de filmes com seus identificadores\n\
    (5) Listar informações de todos os filmes\n\
    (6) Listar informações de um filme específico\n\
    (7) Listar todos os filmes de um determinado gênero\n\
    (8) Sair\n";
    if (send(new_fd, msg, strlen(msg), 0) == -1)
        perror("send");

    
    int opcode = 0;
    char resposta[MAXDATASIZE*2];

    while(opcode != 8)
    {
        printf("server: waiting for client message...\n");
        int numbytes = 0;
        char mensagem_recebida[MAXDATASIZE];
        if ((numbytes = recv(new_fd, mensagem_recebida, MAXDATASIZE, 0)) == -1)
            perror("recv");

        if (numbytes == 0) {
            printf("client disconnected\n");
            break;
        }
        mensagem_recebida[numbytes] = '\0'; // coloca um fim no buffer
        printf("server: received '%s'\n", mensagem_recebida);
        memset(resposta, 0, sizeof(resposta)); // anula a resposta
        snprintf(resposta, sizeof(resposta), "recebido: %s", mensagem_recebida);     // adiciona cabeçalho na resposta

        // tenta obter um numero de resposta logo no começo da string. Se não conseguir envia uma mensagem de erro
        if (sscanf(mensagem_recebida, "%d", &opcode) != 1) {
            snprintf(resposta, sizeof(resposta), "Erro: comando inválido. Por favor, envie um número correspondente a uma operação.");
            opcode = 0; // reset opcode to avoid unintended operations
            if (send(new_fd, resposta, strlen(resposta), 0) == -1)
                perror("send");
            continue;
        }

        
        switch(opcode)
        {
            case 1:
            // Cadastrar um novo filme
            option1(new_fd);

            break;
            case 2:
            // Adicionar um novo gênero a um filme

            strncpy(resposta, "Recebido '2', Adicionando genero de filme.", sizeof(resposta) - 1);
            resposta[sizeof(resposta) - 1] = '\0'; // Ensure null-termination

            if (send(new_fd, resposta, strlen(resposta), 0) == -1)
                    perror("send");

            break;
            case 3:
            // Remover um filme pelo identificador

            strncpy(resposta, "Recebido '3', Removendo um filme pelo identificador.", sizeof(resposta) - 1);
            resposta[sizeof(resposta) - 1] = '\0'; // Ensure null-termination

            if (send(new_fd, resposta, strlen(resposta), 0) == -1)
                    perror("send");

            break;
            case 4:
            //

            strncpy(resposta, "Recebido '4', Listando todos os títulos de filmes com seus identificadores .", sizeof(resposta) - 1);
            resposta[sizeof(resposta) - 1] = '\0'; // Ensure null-termination

            if (send(new_fd, resposta, strlen(resposta), 0) == -1)
                    perror("send");

            break;
            case 5:
            //

            strncpy(resposta, "Recebido '5', Listando informações de todos os filmes.", sizeof(resposta) - 1);
            resposta[sizeof(resposta) - 1] = '\0'; // Ensure null-termination

            if (send(new_fd, resposta, strlen(resposta), 0) == -1)
                    perror("send");

            break;
            case 6:
            //

            strncpy(resposta, "Recebido '6', Listando informações do filme.", sizeof(resposta) - 1);
            resposta[sizeof(resposta) - 1] = '\0'; // Ensure null-termination

            if (send(new_fd, resposta, strlen(resposta), 0) == -1)
                    perror("send");


            break;
            case 7:
            //

            strncpy(resposta, "Recebido '7', Listar todos os filmes desse gênero.", sizeof(resposta) - 1);
            resposta[sizeof(resposta) - 1] = '\0'; // Ensure null-termination

            if (send(new_fd, resposta, strlen(resposta), 0) == -1)
                    perror("send");

            break;

            case 8:
            // Sair
            strncpy(resposta, "Recebido '8', encerrando acesso.", sizeof(resposta) - 1);
            resposta[sizeof(resposta) - 1] = '\0'; // Ensure null-termination

            if (send(new_fd, resposta, strlen(resposta), 0) == -1)
                    perror("send");
            break;
            default:
                strncpy(resposta, "O número enviado não corresponde a nenhuma operação válida. Por favor, envie um número correspondente a uma operação.", sizeof(resposta) - 1);
                resposta[sizeof(resposta) - 1] = '\0'; // Ensure null-termination
            break;
            
        }
        // if (send(new_fd, resposta, strlen(resposta), 0) == -1)
        //     perror("send");
    }
    

}

int main(void)
{
    int sockfd, new_fd;  // listen on sock_fd, new connection on new_fd
    struct addrinfo hints, *servinfo, *p;
    struct sockaddr_storage their_addr; // connector's address information
    socklen_t sin_size;
    struct sigaction sa;
    int yes=1;
    char s[INET6_ADDRSTRLEN];
    int rv;

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE; // use my IP

    if ((rv = getaddrinfo(NULL, PORT, &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }

    // loop through all the results and bind to the first we can
    for(p = servinfo; p != NULL; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype,
                p->ai_protocol)) == -1) {
            perror("server: socket");
            continue;
        }

        if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes,
                sizeof(int)) == -1) {
            perror("setsockopt");
            exit(1);
        }

        if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(sockfd);
            perror("server: bind");
            continue;
        }

        break;
    }

    freeaddrinfo(servinfo); // all done with this structure

    if (p == NULL)  {
        fprintf(stderr, "server: failed to bind\n");
        exit(1);
    }

    if (listen(sockfd, BACKLOG) == -1) {
        perror("listen");
        exit(1);
    }

    sa.sa_handler = sigchld_handler; // reap all dead processes
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    if (sigaction(SIGCHLD, &sa, NULL) == -1) {
        perror("sigaction");
        exit(1);
    }

    printf("server: waiting for connections...\n");

    while(1) {  // main accept() loop
        sin_size = sizeof their_addr;
        new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size);
        if (new_fd == -1) {
            perror("accept");
            continue;
        }

        inet_ntop(their_addr.ss_family,
            get_in_addr((struct sockaddr *)&their_addr),
            s, sizeof s);
        printf("server: got connection from %s\n", s);

        if (!fork()) { // this is the child process
            close(sockfd); // child doesn't need the listener
            atender(new_fd);
            close(new_fd);
            exit(0);
        }
        close(new_fd);  // parent doesn't need this
    }

    return 0;
}