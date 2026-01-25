#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <string.h>
#include <time.h>

#define BUF_SIZE 128

char shared_buffer[BUF_SIZE];
int counter = 0;
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
    while (1) {
        snprintf(shared_buffer, BUF_SIZE, "Запись № %d", ++counter);
        
        print_time();
        printf("WRITER  | записал: \"%s\"\n", shared_buffer);
        
        sem_post(&sem);
        
        sleep(1);
    }
    return NULL;
}

void* reader_thread(void* arg) {
    pthread_t tid = pthread_self();

    while (1) {
        sem_wait(&sem);
        
        print_time();
        printf("READER  | TID=%lu | получено: \"%s\"\n\n",
               (unsigned long)tid, shared_buffer);
        
        fflush(stdout);
        
    }
    return NULL;
}

int main() {
    pthread_t writer, reader;

    sem_init(&sem, 0, 0);

    pthread_create(&writer, NULL, writer_thread, NULL);
    pthread_create(&reader, NULL, reader_thread, NULL);

    pthread_join(writer, NULL);
    pthread_join(reader, NULL);

    sem_destroy(&sem);
    return 0;
}