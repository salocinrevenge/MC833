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

int get_identificador()
{
    FILE *file = fopen("filmes.csv", "r");
    if (file == NULL) {
        perror("fopen");
        return -1;
    }

    int lines = 0;
    char ch;
    while (!feof(file)) {
        ch = fgetc(file);
        if (ch == '\n') {
            lines++;
        }
    }

    fclose(file);
    return lines;
}

void option1(int new_fd)
{
    char mensagem_recebida[MAXDATASIZE];
    const char *msg = "Você escolheu a opção 1: Cadastrar um novo filme.\n\
    Por favor, digite o titulo do filme que deseja cadastrar.\n";
    if (send(new_fd, msg, strlen(msg), 0) == -1)
        perror("send");

    int numbytes = 0;
    char nome[MAXDATASIZE];
    if ((numbytes = recv(new_fd, nome, MAXDATASIZE, 0)) == -1)
        perror("recv");

    


    const char *msg2 = "Beleza, agora digite o(s) genero(s) do filme.\
    Se houver mais de um, separe por vígulas.\n";
    if (send(new_fd, msg2, strlen(msg), 0) == -1)
        perror("send");

    numbytes = 0;
    char genero[MAXDATASIZE];
    if ((numbytes = recv(new_fd, genero, MAXDATASIZE, 0)) == -1)
        perror("recv");



    const char *msg3 = "Ok, insira agora o diretor.\n";
    if (send(new_fd, msg3, strlen(msg), 0) == -1)
        perror("send");

    numbytes = 0;
    char diretor[MAXDATASIZE];
    if ((numbytes = recv(new_fd, diretor, MAXDATASIZE, 0)) == -1)
        perror("recv");




    const char *msg4 = "E agora, qual foi o ano?\n";
    if (send(new_fd, msg4, strlen(msg4), 0) == -1)
        perror("send");

    int valido = 0;
    int ano_int;
    while(!valido)
    {
        numbytes = 0;
        if ((numbytes = recv(new_fd, mensagem_recebida, MAXDATASIZE, 0)) == -1)
            perror("recv");
        printf("server: received '%s'\n", mensagem_recebida);
        if (sscanf(mensagem_recebida, "%d", &ano_int) != 1) {
            const char *error_msg = "Erro: Ano inválido. Por favor, insira um número.\n";
            if (send(new_fd, error_msg, strlen(error_msg), 0) == -1)
                perror("send");
            continue;
        }
        valido = 1;
    }

    FILE *file = fopen("filmes.csv", "a");
    if (file == NULL) {
        perror("fopen");
        return;
    }

    int identificador = get_identificador();
    if (identificador == -1) {
        perror("get_identificador");
        return;
    }

    fprintf(file, "%d,\"%s\",\"%s\",\"%s\",\"%d\"\n", identificador, nome, genero, diretor, ano_int);
    fclose(file);

    const char *msg5 = " Legal! filme inserido!\n<CONTINUE>";
    if (send(new_fd, msg5, strlen(msg5), 0) == -1)
        perror("send");

    
}

void option2(int new_fd)
{
    char mensagem_recebida[MAXDATASIZE];
    const char *msg = "Você escolheu a opção 2: Adicionar um novo gênero a um filme.\n\
    Por favor, digite o identificador do filme que deseja adicionar o gênero.\n";
    if (send(new_fd, msg, strlen(msg), 0) == -1)
        perror("send");

    int valid = 0;
    int n_identificador = 0;
    int numbytes = 0;
    while(!valid){  // Garante que o identificador é válido
        
        numbytes = 0;
        char identificador[MAXDATASIZE];
        if ((numbytes = recv(new_fd, identificador, MAXDATASIZE, 0)) == -1)
            perror("recv");

        n_identificador = 0;
        // Convert string identificador to integer
        if (sscanf(identificador, "%d", &n_identificador) != 1) {
            const char *error_msg = "Erro: Identificador inválido. Por favor, insira um número.\n";
            if (send(new_fd, error_msg, strlen(error_msg), 0) == -1)
                perror("send");
            continue; // Exit the function if invalid identifier
        }
        // Check if the identifier exists in the file
        FILE *file = fopen("filmes.csv", "r");
        if (file == NULL) {
            const char *error_msg = "Erro: Não foi possível abrir o arquivo de filmes. Problemas técnicos, consulte um ADM :(\n";
            if (send(new_fd, error_msg, strlen(error_msg), 0) == -1)
                perror("send");
            printf("Erro ao abrir o arquivo: %s\n<CONTINUE>", strerror(errno));
            return;
        }
        int found = 0;
        char line[MAXDATASIZE];
        while (fgets(line, sizeof(line), file)) {
            int id;
            if (sscanf(line, "%d,", &id) == 1 && id == n_identificador) {
                found = 1;
                break;
            }
        }
        fclose(file);

        if (!found) {
            const char *error_msg = "Erro: Filme com este identificador não encontrado.\n";
            if (send(new_fd, error_msg, strlen(error_msg), 0) == -1)
                perror("send");
            continue;
        }
        break;

    }


    const char *msg2 = "Beleza, agora digite o(s) genero(s) do filme. Se houver mais de um, separe por vígulas.\n";
    if (send(new_fd, msg2, strlen(msg), 0) == -1)
        perror("send");

    numbytes = 0;
    char genero[MAXDATASIZE];
    if ((numbytes = recv(new_fd, genero, MAXDATASIZE, 0)) == -1)
        perror("recv");

    // Open the file for reading
    FILE *file = fopen("filmes.csv", "r");
    if (file == NULL) {
        perror("fopen");
        return;
    }

    // Open a temporary file for writing
    FILE *temp_file = fopen("temp_filmes.csv", "w");
    if (temp_file == NULL) {
        perror("fopen");
        fclose(file);
        return;
    }

    // Read the file line by line and find the matching identifier
    char line[MAXDATASIZE];
    int found = 0;
    while (fgets(line, sizeof(line), file)) {
        int id;
        if (sscanf(line, "%d,", &id) == 1 && id == n_identificador) {
            found = 1;

            // Parse the line to extract the current genres
            char before_genero[MAXDATASIZE];
            char current_genero[MAXDATASIZE];
            char rest_of_line[MAXDATASIZE];
            sscanf(line, "%*d,\"%[^\"]\",\"%[^\"]\",%[^\n]", before_genero, current_genero, rest_of_line);

            // Concatenate the new genres to the existing ones
            strncat(current_genero, ", ", sizeof(current_genero) - strlen(current_genero) - 1);
            strncat(current_genero, genero, sizeof(current_genero) - strlen(current_genero) - 1);

            // Write the updated line to the temporary file
            fprintf(temp_file, "%d,\"%s\",\"%s\",%s\n", id, before_genero, current_genero, rest_of_line);
        } else {
            // Write the original line to the temporary file
            fputs(line, temp_file);
        }
    }

    fclose(file);
    fclose(temp_file);

    if (!found) {
        const char *error_msg = "Erro: Filme com este identificador não encontrado.\n";
        if (send(new_fd, error_msg, strlen(error_msg), 0) == -1)
            perror("send");
        remove("temp_filmes.csv"); // Clean up the temporary file
        return;
    }

    // Replace the original file with the temporary file
    if (rename("temp_filmes.csv", "filmes.csv") != 0) {
        perror("rename");
        const char *error_msg = "Erro: Não foi possível atualizar o arquivo de filmes.\n";
        if (send(new_fd, error_msg, strlen(error_msg), 0) == -1)
            perror("send");
        return;
    }

    const char *success_msg = "Gênero adicionado com sucesso!<CONTINUE>\n";
    if (send(new_fd, success_msg, strlen(success_msg), 0) == -1)
        perror("send");
}

void option4(int new_fd)
{
    const char *msg = "Você escolheu a opção 5: Listar todos os títulos de filmes com seus identificadores.\n<CONTINUE>";
    if (send(new_fd, msg, strlen(msg), 0) == -1)
        perror("send");

    FILE *file = fopen("filmes.csv", "r");
    if (file == NULL) {
        perror("fopen");
        return;
    }

    char line[MAXDATASIZE];
    while (fgets(line, sizeof(line), file)) {
        int id;
        char titulo[MAXDATASIZE];

        if (sscanf(line, "%d,\"%[^\"]\"", &id, titulo) == 2) {
            char formatted_line[MAXDATASIZE*2];
            snprintf(formatted_line, sizeof(formatted_line), "ID: %d; Titulo: \"%s\"\n<CONTINUE>", id, titulo);
            if (send(new_fd, formatted_line, MAXDATASIZE, 0) == -1)
                perror("send");
        }
    }

    const char *end_msg = "Fim da lista de filmes.\n<CONTINUE>";
    if (send(new_fd, end_msg, strlen(end_msg), 0) == -1)
        perror("send");
    fclose(file);

}

void option5(int new_fd)
{
    char mensagem_recebida[MAXDATASIZE];
    const char *msg = "Recebido '5', Listando informações de todos os filmes.\n<CONTINUE>";
    if (send(new_fd, msg, strlen(msg), 0) == -1)
        perror("send");

    FILE *file = fopen("filmes.csv", "r");
    if (file == NULL) {
        perror("fopen");
        return;
    }

    char line[MAXDATASIZE];
    while (fgets(line, sizeof(line), file)) {
        int id;
        char titulo[MAXDATASIZE], genero[MAXDATASIZE], diretor[MAXDATASIZE], ano[MAXDATASIZE];

        if (sscanf(line, "%d,\"%[^\"]\",\"%[^\"]\",\"%[^\"]\",\"%[^\"]\"", &id, titulo, genero, diretor, ano) == 5) {
            char formatted_line[MAXDATASIZE*4+100];
            snprintf(formatted_line, sizeof(formatted_line), "ID: %d; Titulo: \"%s\"; Genero: \"%s\"; Diretor: \"%s\"; Ano: %s\n<CONTINUE>", id, titulo, genero, diretor, ano);
            if (send(new_fd, formatted_line, MAXDATASIZE, 0) == -1)
                perror("send");
        }
    }

    fclose(file);
}

void option6(int new_fd){
    char mensagem_recebida[MAXDATASIZE];
    const char *msg = "Você escolheu a opção 6: Listar informações de um filme específico.\n\
    Por favor, digite o identificador do filme que deseja listar.\n";
    if (send(new_fd, msg, strlen(msg), 0) == -1)
        perror("send");

    int valid = 0;
    int n_identificador = 0;
    int numbytes = 0;
    while(!valid){  // Garante que o identificador é válido
        
        numbytes = 0;
        char identificador[MAXDATASIZE];
        if ((numbytes = recv(new_fd, identificador, MAXDATASIZE, 0)) == -1)
            perror("recv");

        n_identificador = 0;
        // Convert string identificador to integer
        if (sscanf(identificador, "%d", &n_identificador) != 1) {
            const char *error_msg = "Erro: Identificador inválido. Por favor, insira um número.\n";
            if (send(new_fd, error_msg, MAXDATASIZE, 0) == -1)
                perror("send");
            continue; // Exit the function if invalid identifier
        }
        // Check if the identifier exists in the file
        FILE *file = fopen("filmes.csv", "r");
        if (file == NULL) {
            const char *error_msg = "Erro: Não foi possível abrir o arquivo de filmes. Problemas técnicos, consulte um ADM :(\n";
            if (send(new_fd, error_msg, MAXDATASIZE, 0) == -1)
                perror("send");
            printf("Erro ao abrir o arquivo: %s\n<CONTINUE>", strerror(errno));
            return;
        }
        int found = 0;
        char line[MAXDATASIZE];
        while (fgets(line, sizeof(line), file)) {
            int id;
            if (sscanf(line, "%d,", &id) == 1 && id == n_identificador) {
                found = 1;
                break;
            }
        }
        fclose(file);

        if (!found) {
            const char *error_msg = "Erro: Filme com este identificador não encontrado.\n";
            if (send(new_fd, error_msg, strlen(error_msg), 0) == -1)
                perror("send");
            continue;
        }
        break;
    }

    FILE *file = fopen("filmes.csv", "r");
    if (file == NULL) {
        perror("fopen");
        const char *error_msg = "Erro: Não foi possível abrir o arquivo de filmes.\n";
        if (send(new_fd, error_msg, strlen(error_msg), 0) == -1)
            perror("send");
        return;
    }

    char line[MAXDATASIZE];
    int found = 0;
    while (fgets(line, sizeof(line), file)) {
        int id;
        if (sscanf(line, "%d,", &id) == 1 && id == n_identificador) {
            found = 1;

            // Extract headers from the first line
            rewind(file);
            char headers[MAXDATASIZE];
            if (fgets(headers, sizeof(headers), file)) {
                char *header_tokens[5];
                int i = 0;
                char *token = strtok(headers, ",");
                while (token && i < 5) {
                    // Remove newline character if present
                    char *newline = strchr(token, '\n');
                    if (newline) {
                        *newline = '\0';
                    }
                    header_tokens[i++] = token;
                    token = strtok(NULL, ",");
                }

                // Extract movie details
                char titulo[MAXDATASIZE], genero[MAXDATASIZE], diretor[MAXDATASIZE], ano[MAXDATASIZE];
                if (sscanf(line, "%d,\"%[^\"]\",\"%[^\"]\",\"%[^\"]\",\"%[^\"]\"", &id, titulo, genero, diretor, ano) == 5) {
                    char formatted_line[MAXDATASIZE * 4 + 100];
                    printf("ano: %s|\n", ano);
                    snprintf(formatted_line, sizeof(formatted_line), 
                        "%s: %d\n%s: \"%s\"\n%s: \"%s\"\n%s: \"%s\"\n%s: %s\n<CONTINUE>",
                        header_tokens[0], id,
                        header_tokens[1], titulo,
                        header_tokens[2], genero,
                        header_tokens[3], diretor,
                        header_tokens[4], ano);

                    if (send(new_fd, formatted_line, strlen(formatted_line), 0) == -1)
                        perror("send");
                }
            }
            break;
        }
    }

    fclose(file);

    if (!found) {
        const char *error_msg = "Erro: Filme com este identificador não encontrado.\n";
        if (send(new_fd, error_msg, strlen(error_msg), 0) == -1)
            perror("send");
    }

}

int atender(int new_fd)
{
    
    
    int opcode = 0;
    char resposta[MAXDATASIZE*2];
    const char *msg = "\nOlá, cliente! Você se conectou ao servidor com sucesso!\n\
    Você pode realizar as seguintes operações:\n\
    (1) Cadastrar um novo filme\n\
    (2) Adicionar um novo gênero a um filme\n\
    (3) Remover um filme pelo identificador\n\
    (4) Listar todos os títulos de filmes com seus identificadores\n\
    (5) Listar informações de todos os filmes\n\
    (6) Listar informações de um filme específico\n\
    (7) Listar todos os filmes de um determinado gênero\n\
    (8) Sair\n";
    
    while(opcode != 8)
    {
        if (send(new_fd, msg, strlen(msg), 0) == -1)
            perror("send");

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

            option2(new_fd);

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

            option4(new_fd);

            break;
            case 5:
            //

            option5(new_fd);

            break;
            case 6:
            //

            option6(new_fd);

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