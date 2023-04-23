#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <semaphore.h>
#include <fcntl.h>
#include <sys/mman.h>

#define NUM_GROUPS 4  // количество групп пиратов
#define NUM_AREAS 20   // количество участков на острове

sem_t *parrot_sem;
int *is_find;
int TREASURE_INDEX;

int main() {
    srand(time(NULL));

    TREASURE_INDEX = rand() % NUM_AREAS + 1;
    printf("The treasure had hidden on area %d\n", TREASURE_INDEX);
    printf("[!!!]START SEARCHING[!!!]\n");

    int fd = shm_open("/shared_memory", O_CREAT | O_RDWR, 0666); // создаем общую память
    ftruncate(fd, sizeof(int)); // задаем размер
    is_find = (int*) mmap(NULL, sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0); // отображаем в адресное пространство процесса
    *is_find = 0;

    parrot_sem = sem_open("parrot_sem", O_CREAT, 0666, 0); // создаем семафор для работы с кладом

    pid_t pid = fork();
    if (pid == 0) {
        // дочерний процесс
        execlp("search_treasure.c", "search_treasure", "1", NULL); // запускаем процесс поиска с группой 1
    }

    // родительский процесс
    for (int i = 1; i <= NUM_GROUPS; i++) {
        pid_t pid = fork();
        if (pid == 0) {
            // дочерний процесс
            char str[10];
            sprintf(str, "%d", i);
            execlp("./search_treasure", "search_treasure", str, NULL); // запускаем процесс поиска с группой i
        }
    }

    sem_wait(parrot_sem); // ждем, пока клад не будет найден

    printf("All groups finished searching.\n");
    printf("Treasure found!!!\n");
    printf("All hands on deck!!!\n");

    sem_close(parrot_sem);
    sem_unlink("parrot_sem");
    munmap(is_find, sizeof(int)); // отключаем отображение общей памяти
    shm_unlink("/shared_memory"); // удаляем общую память

    return 0;
}
