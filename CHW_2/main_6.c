#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <semaphore.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/ipc.h>
#include <sys/sem.h>

#define NUM_GROUPS 4  // количество групп пиратов
#define NUM_AREAS 20   // количество участков на острове
#define SEM_KEY 0x1234

int group_sem[NUM_GROUPS];
int parrot_sem;
pthread_t threads[NUM_GROUPS];

int is_find = 0;
int TREASURE_INDEX;

int min(int a, int b) {
    return a < b ? a : b;
}

void sem_wait_parrot() {
    struct sembuf op = {0, -1, 0};
    semop(parrot_sem, &op, 1);
}

void sem_post_parrot() {
    struct sembuf op = {0, 1, 0};
    semop(parrot_sem, &op, 1);
}

// функция для поиска клада на участке
void *search_treasure(void *arg) {
    int group = *(int *) arg;  // номер группы
    int start_area = (group - 1) * (NUM_AREAS /  NUM_GROUPS); // Вычисляем номер участка, с которого начнет искать данная группа
    int end_area = min(start_area + (NUM_AREAS /  NUM_GROUPS),
                       NUM_AREAS); // Вычисляем номер участка, на котором закончит поиск данная группа

    for (int i = start_area; i < end_area; i++) {
        if (is_find) {
            break;
        }
        printf("Group %d searches area %d\n", group, i);
        sleep(1);  // имитация поиска клада
        if (i == TREASURE_INDEX && !is_find) {
            is_find = 1;
            printf("[!!!]Group %d found the treasure in area %d[!!!]\n", group, i);
            sem_post_parrot();
            break;
        }
    }

    printf("Group %d finished searching.\n", group);
    return NULL;
}

void sigint_handler(int signo) {
    printf("Program completed\n");
    for (int i = 0; i < NUM_GROUPS; i++) {
        pthread_join(threads[i], NULL);  // ожидаем завершения работы потоков
    }

    sem_destroy(&parrot_sem);

    for (int i = 0; i < NUM_GROUPS; ++i) {
        sem_destroy(&group_sem[i]);
    }

    exit(0);
}

int main() {
    signal(SIGINT, sigint_handler);
    srand(time(NULL));

    int groups[NUM_GROUPS];
    TREASURE_INDEX = rand() % NUM_AREAS + 1;
    printf("The treasure had hidden on area %d\n", TREASURE_INDEX);
    printf("[!!!]START SEARCHING[!!!]\n");

    parrot_sem = semget(SEM_KEY, 0, IPC_CREAT | 0666);

    for (int i = 0; i < NUM_GROUPS; ++i) {
        group_sem[i] = semget(SEM_KEY, 0, IPC_CREAT | 0666);
    }

    for (int i = 0; i < NUM_GROUPS; i++) {
        groups[i] = i + 1;
        pthread_create(&threads[i], NULL, search_treasure, &groups[i]);  // создаем потоки для каждой группы
    }

    sem_wait_parrot();

    for (int i = 0; i < NUM_GROUPS; i++) {
        pthread_join(threads[i], NULL);  // ожидаем завершения работы потоков
    }

    semctl(parrot_sem, 0, IPC_RMID, 0);

    for (int i = 0; i < NUM_GROUPS; ++i) {
        semctl(group_sem[i], 0, IPC_RMID, 0);
    }

    printf("All groups finished searching.\n");
    printf("Treasure found!!!\n");
    printf("All hands on deck!!!\n");
}
