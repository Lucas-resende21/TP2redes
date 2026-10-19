#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define PORT 8080
#define BUF_SIZE 256

void *atende_cliente(void *arg) {
    int client_fd = (int)(long)arg;
    unsigned long tid = (unsigned long)pthread_self();

    struct sockaddr_in caddr;
    socklen_t len = sizeof(caddr);
    getpeername(client_fd, (struct sockaddr *)&caddr, &len);
    char *ip = inet_ntoa(caddr.sin_addr);
    int porta = ntohs(caddr.sin_port);

    printf("[Thread %lu] Atendendo cliente %s na porta %d\n", tid, ip, porta);

    char buf[BUF_SIZE] = {0};
    ssize_t n = recv(client_fd, buf, sizeof(buf) - 1, 0);
    if (n > 0) {
        printf("[Thread %lu] Recebido de %s:%d: %s\n", tid, ip, porta, buf);
        const char *resp = "Mensagem recebida pelo servidor TCP!";
        send(client_fd, resp, strlen(resp), 0);
    }
    printf("[Thread %lu] Segurando a conexão aberta\n", tid);
    while(1);
    close(client_fd);
    return NULL;
}

int main(void) {
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        perror("socket");
        return 1;
    }

    /* Permite reiniciar o servidor rapidamente sem erro de "endereço em uso" */
    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("bind");
        close(server_fd);
        return 1;
    }

    if (listen(server_fd, 5) < 0) {
        perror("listen");
        close(server_fd);
        return 1;
    }

    printf("[TCP] Servidor escutando na porta %d...\n", PORT);

    while (1) {
        int client_fd = accept(server_fd, NULL, NULL);
        if (client_fd < 0) {
            perror("accept");
            continue;
        }
        printf("[TCP] Cliente conectado.\n");

        /* Cria uma thread nova para esse cliente e volta a esperar o próximo */
        pthread_t tid;
        pthread_create(&tid, NULL, atende_cliente, (void *)(long)client_fd);
        pthread_detach(tid);
    }

    close(server_fd);
    return 0;
}