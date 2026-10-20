#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define BUF_SIZE 256

int main(int argc, char *argv[]) {
    const char *ip = argv[1];
    int PORT = strtol(argv[2], NULL, 10);
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("socket");
        return 1;
    }

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(PORT);
    inet_pton(AF_INET, ip, &addr.sin_addr);

    if (connect(sockfd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("connect");
        close(sockfd);
        return 1;
    }

    const char *msg = "Ola, servidor TCP!";
    send(sockfd, msg, strlen(msg), 0);
    printf("[TCP] Mensagem enviada: %s\n", msg);

    char buf[BUF_SIZE] = {0};
    ssize_t n = recv(sockfd, buf, sizeof(buf) - 1, 0);
    if (n > 0) {
        printf("[TCP] Resposta do servidor: %s\n", buf);
    }

    printf("Enrolando pra manter a conexão aberta.\n");
    while(1);
    close(sockfd);
    return 0;
}
