#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <unistd.h>
#include <semaphore.h>
#include <sys/mman.h>
#include <pthread.h>
#include <time.h>

char curr_stat = 0;
const char stat_ok = 1;
const char stat_finish = 0;

#define NUM_AREAS 20   // количество участков на острове

int socketfd;
sem_t parrot_sem;
char TREASURE_INDEX[3];
int NUM_GROUPS;  // количество групп пиратов

struct sockaddr_in server_addr;

int trySocket(int domain, int type, int protocol) {
    int res = socket(domain, type, protocol);

    if (res == -1) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }
    return res;
}

void Inet_aton(const char *src, void *dst) {
    int res = inet_aton(src, dst);

    if (res == 0) {
        printf("inet_aton failed");
        exit(EXIT_FAILURE);
    }
    if (res == -1) {
        perror("inet_aton failed");
        exit(EXIT_FAILURE);
    }
}

void sendNewData() {
    socklen_t addr_len = sizeof(server_addr);
    sendto(socketfd, TREASURE_INDEX, 3, 0, (struct sockaddr *) &server_addr, addr_len);
}

int min(int a, int b) {
    return a < b ? a : b;
}

// функция для поиска клада на участке
void *search_treasure(void *arg) {
    int group = *(int *) arg;  // номер группы
    int start_area = (group - 1) * (NUM_AREAS / NUM_GROUPS); // Вычисляем номер участка, с которого начнет искать данная группа
    int end_area = min(start_area * (group), NUM_AREAS); // Вычисляем номер участка, на котором закончит поиск данная группа

    for (int i = start_area; i <= end_area; i++) {
        if (TREASURE_INDEX[1]) {
            break;
        }
        printf("Group %d searches area %d\n", group, i + 1);
        sleep(1);  // имитация поиска клада
        if (i == TREASURE_INDEX[0] && !TREASURE_INDEX[1]) {
            TREASURE_INDEX[1] = 1;
            printf("[!!!]Group %d found the treasure in area %d[!!!]\n", group, i + 1);
            sem_post(&parrot_sem);

            sendNewData();
        }
    }
    sendNewData();
    printf("Group %d finished searching.\n", group);

    return NULL;
}

int main(int argc, char **argv) {
    srand(time(NULL));

    socketfd = trySocket(AF_INET, SOCK_DGRAM, 0);
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(12345);
    Inet_aton("127.0.0.1", &server_addr.sin_addr);

    socklen_t addr_len = sizeof(server_addr);
    sendto(socketfd, &stat_ok, 1, 0, (struct sockaddr *) &server_addr, addr_len);
    recvfrom(socketfd, TREASURE_INDEX, 3, 0, (struct sockaddr *) &server_addr,
             &addr_len);

    NUM_GROUPS = TREASURE_INDEX[2];

    sem_t group_sem[NUM_GROUPS];
    sem_init(&parrot_sem, 0, 0);
    for (int i = 0; i < NUM_GROUPS; ++i) {
        sem_init(&group_sem[i], 0, 0);
    }

    int groups[NUM_GROUPS];
    pthread_t threads[NUM_GROUPS];
    for (int i = 0; i < NUM_GROUPS; i++) {
        groups[i] = i + 1;
        pthread_create(&threads[i], NULL, search_treasure, &groups[i]);  // создаем потоки для каждой группы
    }


    sem_wait(&parrot_sem);

    for (int i = 0; i < NUM_GROUPS; i++) {
        pthread_join(threads[i], NULL);  // ожидаем завершения работы потоков
    }
    sem_destroy(&parrot_sem);

    for (int i = 0; i < NUM_GROUPS; ++i) {
        sem_destroy(&group_sem[i]);
    }

    close(socketfd);
    printf("All groups finished searching.\n");
    printf("Treasure found!!!\n");
    printf("All hands on deck!!!\n");
    return 0;
}

