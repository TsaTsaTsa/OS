#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <unistd.h>
#include <time.h>

#define NUM_GROUPS 4  // количество групп пиратов
#define NUM_AREAS 20   // количество участков на острове

char curr_stat = 0;
const char stat_ok = 1;
const char stat_finish = 0;

int trySocket(int domain, int type, int protocol) {
    if (socket(domain, type, protocol) == -1) {
        perror("[!ERROR] Socket Faild");
        exit(1);
    }
    return socket(domain, type, protocol);
}

void tryBind(int sockfd, const struct sockaddr *addr, socklen_t addrlen) {
    if (bind(sockfd, addr, addrlen) == -1) {
        perror("[!ERROR] tryBind Fail");
        exit(1);
    }
}

int main(int argc, char **argv) {
    srand(time(NULL));

    int socketfd = trySocket(AF_INET, SOCK_DGRAM, 0);

    struct sockaddr_in server_addr, client_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(12345);
    server_addr.sin_addr.s_addr = INADDR_ANY;
    char TREASURE_INDEX[3] = {rand() % NUM_AREAS + 1, 0, NUM_GROUPS};

    tryBind(socketfd, (struct sockaddr *) &server_addr, sizeof(server_addr));

    socklen_t addr_len = sizeof(client_addr);

    recvfrom(socketfd, &curr_stat, 1, 0, (struct sockaddr *) &client_addr,
             &addr_len);

    sendto(socketfd, TREASURE_INDEX, 3, 0, (struct sockaddr *) &client_addr,
           addr_len);
    recvfrom(socketfd, &curr_stat, 3, 0, (struct sockaddr *) &client_addr, &addr_len);

    ssize_t nread;
    while (!TREASURE_INDEX[1]) {
        nread = recvfrom(socketfd, TREASURE_INDEX, 3, 0, (struct sockaddr *) &client_addr, &addr_len);
        printf("Upload server\n");

        sendto(socketfd, &stat_ok, 1, 0, (struct sockaddr *) &client_addr, addr_len);

        if (nread == -1) {
            perror("[!ERROR] Read crush\n");
            exit(EXIT_FAILURE);
        }
        if (nread == 0) {
            printf("[!ERROR] Crush read file\n");
        }
    }

    sendto(socketfd, &stat_finish, 1, 0, (struct sockaddr *) &client_addr, addr_len);

    close(socketfd);

    return 0;
} 