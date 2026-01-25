#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>

#define READERS 10
#define BUF_SIZE 64

char shared_buf[BUF_SIZE];
int counter = 0;

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

void *writer_thread(void *arg) {
    (void)arg;

    while (1) {
        pthread_mutex_lock(&mutex);

        counter++;
        snprintf(shared_buf, BUF_SIZE,
                 "counter=%d",
                 counter);

        printf("[WRITER] записал %d\n", counter);

        pthread_mutex_unlock(&mutex);

        sleep(1);
    }
    return NULL;
}


void *reader_thread(void *arg) {
    long id = (long)arg;

    while (1) {
        pthread_mutex_lock(&mutex);

        printf("  [READER %ld | TID=%lu] видит: %s\n",
               id,
               (unsigned long)pthread_self(),
               shared_buf);

        pthread_mutex_unlock(&mutex);

        sleep(1);
    }
    return NULL;
}


int main(void) {
    pthread_t writer;
    pthread_t readers[READERS];

    if (pthread_create(&writer, NULL, writer_thread, NULL) != 0) {
        perror("pthread_create writer");
        exit(1);
    }

    for (long i = 0; i < READERS; i++) {
        if (pthread_create(&readers[i],
                           NULL,
                           reader_thread,
                           (void *)i) != 0) {
            perror("pthread_create reader");
            exit(1);
        }
    }

    pthread_join(writer, NULL);

    for (int i = 0; i < READERS; i++) {
        pthread_join(readers[i], NULL);
    }

    return 0;
}
