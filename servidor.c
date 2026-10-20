#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define BUF_SIZE 256

int num_mesa;
int num_clientes = 0;
pthread_mutex_t trava = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t mesa_livre = PTHREAD_COND_INITIALIZER;

void *atende_cliente(void *arg) {
    int client_fd = (int)(long)arg;
    unsigned long tid = (unsigned long)pthread_self();

    struct sockaddr_in caddr;
    socklen_t len = sizeof(caddr);
    getpeername(client_fd, (struct sockaddr *)&caddr, &len);
    char ip[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &caddr.sin_addr, ip, sizeof(ip));
    int porta = ntohs(caddr.sin_port);

    printf("[Thread %lu] Atendendo cliente %s na porta %d\n", tid, ip, porta);

    pthread_mutex_lock(&trava);
    if (num_clientes >= num_mesa) {
        printf("[Thread %lu] Cliente %s:%d esperando mesa\n", tid, ip, porta);
        const char *espera = "Esperando mesa\n";
        send(client_fd, espera, strlen(espera), 0);
        while (num_clientes >= num_mesa)
            pthread_cond_wait(&mesa_livre, &trava);
    }
    num_clientes++;
    printf("[Thread %lu] Cliente %s:%d conseguiu uma mesa (%d/%d ocupadas)\n",
           tid, ip, porta, num_clientes, num_mesa);
    pthread_mutex_unlock(&trava);

    const char *ok = "Consegui uma mesa\n";
    send(client_fd, ok, strlen(ok), 0);

    char buf[BUF_SIZE];
    while (recv(client_fd, buf, sizeof(buf), 0) > 0);

    pthread_mutex_lock(&trava);
    num_clientes--;
    printf("[Thread %lu] Cliente %s:%d saiu, mesa liberada (%d/%d ocupadas)\n",
           tid, ip, porta, num_clientes, num_mesa);
    pthread_cond_signal(&mesa_livre);
    pthread_mutex_unlock(&trava);

    close(client_fd);
    return NULL;
}

int main(int argc, char *argv[]) {
    if (argc < 3) {
        printf("Uso: %s <porta> <num_mesa>\n", argv[0]);
        return 1;
    }
    int PORT_client = strtol(argv[1], NULL, 10);
    num_mesa = strtol(argv[2], NULL, 10);
    printf("[TCP] Servidor iniciado com %d mesas\n", num_mesa);

    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        perror("socket");
        return 1;
    }

    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(PORT_client);

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

    printf("[TCP] Servidor escutando na porta %d...\n", PORT_client);

    while (1) {
        int client_fd = accept(server_fd, NULL, NULL);
        if (client_fd < 0) {
            perror("accept");
            continue;
        }
        printf("[TCP] Cliente conectado.\n");

        pthread_t tid;
        pthread_create(&tid, NULL, atende_cliente, (void *)(long)client_fd);
        pthread_detach(tid);
    }

    close(server_fd);
    return 0;
}