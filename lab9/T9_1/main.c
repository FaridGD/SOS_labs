#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <string.h>
#include <time.h>

#define BUF_SIZE 128

char shared_buffer[BUF_SIZE];
sem_t sem;

void print_time() {
    time_t now = time(NULL);
    struct tm *tm_info = localtime(&now);
    printf("[%02d:%02d:%02d] ",
           tm_info->tm_hour,
           tm_info->tm_min,
           tm_info->tm_sec);
}

void* writer_thread(void* arg) {
    int counter = 1;

    while (1) {
        sem_wait(&sem);

        print_time();
        printf("WRITER  | начинает запись...\n");

        snprintf(shared_buffer, BUF_SIZE, "Запись № %d", counter++);
        sleep(1);  // имитируем длительную запись

        print_time();
        printf("WRITER  | записал: \"%s\"\n\n", shared_buffer);

        sem_post(&sem);
        fflush(stdout);

        sleep(1);
    }
    return NULL;
}

void* reader_thread(void* arg) {
    pthread_t tid = pthread_self();

    while (1) {
        sem_wait(&sem);

        print_time();
        printf("READER  | TID=%lu | читает данные...\n",
               (unsigned long)tid);

        print_time();
        printf("READER  | получено: \"%s\"\n\n", shared_buffer);

        sem_post(&sem);
        fflush(stdout);

        sleep(1);
    }
    return NULL;
}

int main() {
    pthread_t writer, reader;

    sem_init(&sem, 0, 1);

    pthread_create(&writer, NULL, writer_thread, NULL);
    pthread_create(&reader, NULL, reader_thread, NULL);

    pthread_join(writer, NULL);
    pthread_join(reader, NULL);

    sem_destroy(&sem);
    return 0;
}
