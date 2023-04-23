#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <fcntl.h>
#include <sys/mman.h>

#define NUM_AREAS 20   // количество участков на острове

sem_t *parrot_sem;
int *is_find;
int TREASURE_INDEX;

void search_treasure(int group_num) {
    srand(time(NULL) + group_num);
    int current_area = rand() % NUM_AREAS + 1;

    if (current_area == TREASURE_INDEX) {
        sem_wait(parrot_sem); // ждем, пока можно захватить семафор
        (*is_find)++; // увеличиваем счетчик клада
        printf("Group %d found the treasure in area %d.\n", group_num, current_area);
        sem_post(parrot_sem); // освобождаем семафор
    }

    exit(0);
}

int main(int argc, char *argv[]) {
    int group_num = atoi(argv[1]);

    int fd = shm_open("/shared_memory", O_RDWR, 0666); // получаем дескриптор общей памяти
    is_find = (int*) mmap(NULL, sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0); // отображаем в адресное пространство процесса
    parrot_sem = sem_open("parrot_sem", 0); // получаем дескриптор семафора

    search_treasure(group_num);

    sem_close(parrot_sem);
    munmap(is_find, sizeof(int)); // отключаем отображение общей памяти

    return 0;
}
