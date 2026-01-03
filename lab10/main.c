#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>

#define READERS 10
#define BUF_SIZE 64

char shared_buffer[BUF_SIZE];
int counter = 0;

pthread_rwlock_t rwlock;

void* writer(void* arg) {
    while (1) {
        pthread_rwlock_wrlock(&rwlock);

        counter++;
        snprintf(shared_buffer, BUF_SIZE, "Запись номер %d", counter);

        printf("Writer: обновил данные\n");

        pthread_rwlock_unlock(&rwlock);

        sleep(1);
    }
    return NULL;
}

void* reader(void* arg) {
    long tid = (long)arg;

    while (1) {
        pthread_rwlock_rdlock(&rwlock);

        printf("Reader %ld: buffer = \"%s\"\n", tid, shared_buffer);

        pthread_rwlock_unlock(&rwlock);

        sleep(1);
    }
    return NULL;
}

int main() {
    pthread_t readers[READERS];
    pthread_t writer_thread;

    pthread_rwlock_init(&rwlock, NULL);

    memset(shared_buffer, 0, BUF_SIZE);

    for (long i = 0; i < READERS; i++) {
        pthread_create(&readers[i], NULL, reader, (void*)i);
    }

    pthread_create(&writer_thread, NULL, writer, NULL);

    pthread_join(writer_thread, NULL);
    for (int i = 0; i < READERS; i++) {
        pthread_join(readers[i], NULL);
    }

    pthread_rwlock_destroy(&rwlock);

    return 0;
}

