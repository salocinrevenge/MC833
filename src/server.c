#include "base_connection.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <signal.h>
#include <pthread.h>

#define BACKLOG 10   // how many pending connections queue will hold

pthread_mutex_t file_mutex;


int count_lines(FILE *file)
{
    int lines = 0;
    char ch;
    while ((ch = fgetc(file)) != EOF)
        if (ch == '\n')
            lines++;

    rewind(file);
    return lines;
}

int get_valid_identifier(int fd) {
    int n_identificador;
    int valid = 0;
    while (!valid) {  // Garante que o identificador é válido
        char *identificador;
        recv_message(fd, &identificador);

        // Convert string identificador to integer
        if (sscanf(identificador, "%d", &n_identificador) != 1) {
            const char *error_msg = "Erro: Identificador inválido. Por favor, insira um número.\n";
            send_message(fd, error_msg, 0);
            free(identificador);
            continue;
        }
        free(identificador);

        // Check if the identifier exists in the file
        FILE *file = fopen("filmes.csv", "r");
        if (file == NULL) {
            const char *error_msg = "Erro: Não foi possível abrir o arquivo de filmes. Problemas técnicos, consulte um ADM :(\n";
            send_message(fd, error_msg, 0);
            printf("Erro ao abrir o arquivo: %s\n", strerror(errno));
            return -1;
        }
        int id, found = 0;
        char line[MAXDATASIZE];
        while (fgets(line, sizeof(line), file)) {
            if (sscanf(line, "%d,", &id) == 1 && id == n_identificador) {
                found = 1;
                break;
            }
        }
        fclose(file);

        if (!found) {
            const char *error_msg = "Erro: Filme com este identificador não encontrado.\n";
            send_message(fd, error_msg, 0);
            continue;
        }
        valid = 1;
    }
    return n_identificador;
}

void option1(int new_fd)
{
    const char *msg = "Você escolheu a opção 1: Cadastrar um novo filme.\n\
    Por favor, digite o titulo do filme que deseja cadastrar.\n";
    send_message(new_fd, msg, 0);

    char *nome;
    recv_message(new_fd, &nome);

    // Get movie genre(s)
    const char *msg2 = "Beleza, agora digite o(s) gênero(s) do filme.\
    Se houver mais de um, separe por vírgulas.\n";
    send_message(new_fd, msg2, 0);

    char *genero;
    recv_message(new_fd, &genero);

    // Get movie director
    const char *msg3 = "Ok, insira agora o diretor.\n";
    send_message(new_fd, msg3, 0);

    char *diretor;
    recv_message(new_fd, &diretor);

    // Get movie year
    const char *msg4 = "E agora, qual foi o ano?\n";
    send_message(new_fd, msg4, 0);

    int valido = 0;
    int ano_int;
    char *ano;
    while (!valido)
    {
        recv_message(new_fd, &ano);
        printf("server: received '%s'\n", ano);
        if (sscanf(ano, "%d", &ano_int) != 1) {
            const char *error_msg = "Erro: Ano inválido. Por favor, insira um número.\n";
            send_message(new_fd, error_msg, 0);
            free(ano);
            continue;
        }
        valido = 1;
    }

    pthread_mutex_lock(&file_mutex);
    FILE *file = fopen("filmes.csv", "a+");
    if (file == NULL) {
        perror("fopen");
        return;
    }

    int identificador = count_lines(file);
    if (identificador == -1) {
        perror("count_lines");
        return;
    }

    fprintf(file, "%d,\"%s\",\"%s\",\"%s\",\"%d\"\n", identificador, nome, genero, diretor, ano_int);
    fclose(file);
    pthread_mutex_unlock(&file_mutex);

    const char *msg5 = " Legal! Filme inserido!\n";
    send_message(new_fd, msg5, 1);
    
    free(nome);
    free(genero);
    free(diretor);
    free(ano);
}

void option2(int new_fd)
{
    const char *msg = "Você escolheu a opção 2: Adicionar um novo gênero a um filme.\n\
    Por favor, digite o identificador do filme que deseja adicionar o gênero.\n";
    send_message(new_fd, msg, 0);

    int n_identificador = get_valid_identifier(new_fd);
    if (n_identificador == -1)
        return;

    const char *msg2 = "Beleza, agora digite o(s) gênero(s) que deseja adicionar do filme. Se houver mais de um, separe por vírgulas.\n";
    send_message(new_fd, msg2, 0);

    char *genero;
    recv_message(new_fd, &genero);

    // Open the file for reading
    FILE *file = fopen("filmes.csv", "r");
    if (file == NULL) {
        perror("fopen");
        free(genero);
        return;
    }

    // Open a temporary file for writing
    FILE *temp_file = fopen("temp_filmes.csv", "w");
    if (temp_file == NULL) {
        perror("fopen");
        fclose(file);
        free(genero);
        return;
    }

    // Read the file line by line and find the matching identifier
    char line[MAXDATASIZE];
    int id, found = 0;
    while (fgets(line, sizeof(line), file)) {
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
    free(genero);

    if (!found) {
        const char *error_msg = "Erro: Filme com este identificador não encontrado.\n";
        send_message(new_fd, error_msg, 0);
        remove("temp_filmes.csv"); // Clean up the temporary file
        return;
    }

    // Replace the original file with the temporary file
    if (rename("temp_filmes.csv", "filmes.csv") != 0) {
        perror("rename");
        const char *error_msg = "Erro: Não foi possível atualizar o arquivo de filmes.\n";
        send_message(new_fd, error_msg, 0);
        return;
    }

    const char *success_msg = "Gênero adicionado com sucesso!\n";
    send_message(new_fd, success_msg, 1);
}

void option3(int new_fd)
{
    const char *msg = "Você escolheu a opção 3: Remover um filme pelo identificador.\n\
    Por favor, digite o identificador do filme que deseja remover.\n";
    send_message(new_fd, msg, 0);

    int n_identificador = get_valid_identifier(new_fd);
    if (n_identificador == -1)
        return;

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

    // Write every line except the one with the matching id to the temporary file
    char line[MAXDATASIZE];
    int id, found = 0;
    while (fgets(line, sizeof(line), file)) {
        if (!(sscanf(line, "%d,", &id) == 1 && id == n_identificador)) {
            fputs(line, temp_file);
        }
    }

    fclose(file);
    fclose(temp_file);

    // Replace the original file with the temporary file
    if (rename("temp_filmes.csv", "filmes.csv") != 0) {
        perror("rename");
        const char *error_msg = "Erro: Não foi possível atualizar o arquivo de filmes.\n";
        send_message(new_fd, error_msg, 0);
        return;
    }

    const char *success_msg = "Filme removido com sucesso!\n";
    send_message(new_fd, success_msg, 1);
}

void option4(int new_fd)
{
    const char *msg = "Você escolheu a opção 4: Listar todos os títulos de filmes com seus identificadores.\n";
    send_message(new_fd, msg, 1);

    FILE *file = fopen("filmes.csv", "r");
    if (file == NULL) {
        perror("fopen");
        return;
    }

    int id;
    char line[MAXDATASIZE];
    char titulo[MAXDATASIZE];
    while (fgets(line, sizeof(line), file)) {
        if (sscanf(line, "%d,\"%[^\"]\"", &id, titulo) == 2) {
            char formatted_line[MAXDATASIZE*2];
            snprintf(formatted_line, sizeof(formatted_line), "ID: %d; Titulo: \"%s\"\n", id, titulo);
            send_message(new_fd, formatted_line, 1);
        }
    }

    const char *end_msg = "Fim da lista de filmes.\n";
    send_message(new_fd, end_msg, 1);
    fclose(file);
}

void option5(int new_fd)
{
    const char *msg = "Você escolheu a opção 5: Listar informações de todos os filmes.\n";
    send_message(new_fd, msg, 1);

    FILE *file = fopen("filmes.csv", "r");
    if (file == NULL) {
        perror("fopen");
        return;
    }

    char line[MAXDATASIZE];
    int id;
    char titulo[MAXDATASIZE], genero[MAXDATASIZE], diretor[MAXDATASIZE], ano[MAXDATASIZE];
    while (fgets(line, sizeof(line), file)) {
        if (sscanf(line, "%d,\"%[^\"]\",\"%[^\"]\",\"%[^\"]\",\"%[^\"]\"", &id, titulo, genero, diretor, ano) == 5) {
            char formatted_line[MAXDATASIZE*4+100];
            snprintf(formatted_line, sizeof(formatted_line),
                "ID: %d; Titulo: \"%s\"; Genero: \"%s\"; Diretor: \"%s\"; Ano: %s\n",
                id, titulo, genero, diretor, ano);
            send_message(new_fd, formatted_line, 1);
        }
    }

    fclose(file);
}

void option6(int new_fd){
    const char *msg = "Você escolheu a opção 6: Listar informações de um filme específico.\n\
    Por favor, digite o identificador do filme que deseja listar.\n";
    send_message(new_fd, msg, 0);

    int n_identificador = get_valid_identifier(new_fd);
    if (n_identificador == -1)
        return;

    FILE *file = fopen("filmes.csv", "r");
    if (file == NULL) {
        perror("fopen");
        const char *error_msg = "Erro: Não foi possível abrir o arquivo de filmes.\n";
        send_message(new_fd, error_msg, 0);
        return;
    }

    char line[MAXDATASIZE];
    int id;
    while (fgets(line, sizeof(line), file)) {
        if (sscanf(line, "%d,", &id) == 1 && id == n_identificador) {
            // Extract movie details
            char titulo[MAXDATASIZE], genero[MAXDATASIZE], diretor[MAXDATASIZE], ano[MAXDATASIZE];
            if (sscanf(line, "%*d,\"%[^\"]\",\"%[^\"]\",\"%[^\"]\",\"%[^\"]\"", titulo, genero, diretor, ano) == 4) {
                char formatted_line[MAXDATASIZE * 4 + 100];
                snprintf(formatted_line, sizeof(formatted_line),
                    "ID: %d; Titulo: \"%s\"; Genero: \"%s\"; Diretor: \"%s\"; Ano: %s\n",
                    id, titulo, genero, diretor, ano);
                send_message(new_fd, formatted_line, 1);
            }
            break;
        }
    }

    fclose(file);
}

void option7(int new_fd)
{
    const char *msg = "Você escolheu a opção 7: Listar todos os filmes de um determinado gênero.\n\
    Por favor, digite o gênero que você deseja buscar.\n";
    send_message(new_fd, msg, 0);

    char *genero;
    recv_message(new_fd, &genero);

    // Open the file for reading
    FILE *file = fopen("filmes.csv", "r");
    if (file == NULL) {
        perror("fopen");
        free(genero);
        return;
    }

    // Send an initial message
    const char *count_msg = "Buscando filmes...\n";
    send_message(new_fd, count_msg, 1);

    int id, found = 0;
    char line[MAXDATASIZE];
    char movie_title[MAXDATASIZE];
    char current_genero[MAXDATASIZE];
    while (fgets(line, sizeof(line), file)) {
        if (sscanf(line, "%d,\"%[^\"]\",\"%[^\"]\",%*[^\n]", &id, movie_title, current_genero) == 3 &&
            strstr(current_genero, genero)) {                
                char formatted_line[MAXDATASIZE*2];
                snprintf(formatted_line, sizeof(formatted_line), "ID: %d; Titulo: \"%s\"\n", id, movie_title);
                send_message(new_fd, formatted_line, 1);
                found++;
        }
    }

    fclose(file);
    free(genero);

    // Send the final message
    char *end_msg;
    if (found > 0) {
        end_msg = "Fim da lista de filmes.\n";
    } else {
        end_msg = "Nenhum filme com este gênero foi encontrado.\n";
    }
    send_message(new_fd, end_msg, 1);
}

void atender(int new_fd)
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
        send_message(new_fd, msg, 0);

        printf("server: waiting for client message...\n");
        char *mensagem_recebida;

        retorno_recv numbytes = recv_message(new_fd, &mensagem_recebida);

        if (numbytes.len == 0) {
            printf("client disconnected\n");
            free(mensagem_recebida);
            break;
        }
        printf("server: received '%s'\n", mensagem_recebida);
        memset(resposta, 0, sizeof(resposta)); // anula a resposta
        snprintf(resposta, sizeof(resposta), "recebido: %s", mensagem_recebida);     // adiciona cabeçalho na resposta

        // tenta obter um numero de resposta logo no começo da string. Se não conseguir envia uma mensagem de erro
        if (sscanf(mensagem_recebida, "%d", &opcode) != 1) {
            snprintf(resposta, sizeof(resposta), "Erro: comando inválido. Por favor, envie um número correspondente a uma operação.");
            send_message(new_fd, resposta, 0);
            opcode = 0; // reset opcode to avoid unintended operations
            free(mensagem_recebida);
            continue;
        }
        free(mensagem_recebida);

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
                option3(new_fd);
                break;
            case 4:
                // Listar todos os títulos de filmes com seus identificadores
                option4(new_fd);
                break;
            case 5:
                // Listar informações de todos os filmes
                option5(new_fd);
                break;
            case 6:
                // Listar informações de um filme específico
                option6(new_fd);
                break;
            case 7:
                // Listar todos os filmes de um determinado gênero
                option7(new_fd);
                break;
            case 8:
                // Sair
                strncpy(resposta, "Recebido '8', encerrando acesso.", sizeof(resposta) - 1);
                resposta[sizeof(resposta) - 1] = '\0'; // Ensure null-termination
                send_message(new_fd, resposta, 0);
                break;
            default:
                strncpy(resposta, "O número enviado não corresponde a nenhuma operação válida. Por favor, envie um número correspondente a uma operação.", sizeof(resposta) - 1);
                resposta[sizeof(resposta) - 1] = '\0'; // Ensure null-termination
                send_message(new_fd, resposta, 0);
                break;
        }
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

    pthread_mutex_destroy(&file_mutex);
    return 0;
}