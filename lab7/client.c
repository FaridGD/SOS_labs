#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <time.h>

#define SHM_NAME "/time_shm"

typedef struct {
    char time_str[64];
    pid_t sender_pid;
} shared_data;

int main(void) {
    printf("Клиент запущен (PID: %d)\n", getpid());

    int shm_fd = shm_open(SHM_NAME, O_RDONLY, 0666);
    if (shm_fd == -1) {
        perror("shm_open");
        return 1;
    }

    shared_data *data = mmap(NULL,
                             sizeof(shared_data),
                             PROT_READ,
                             MAP_SHARED,
                             shm_fd,
                             0);
    if (data == MAP_FAILED) {
        perror("mmap");
        return 1;
    }

    while (1) {
        time_t now = time(NULL);
        struct tm *tm_info = localtime(&now);
        char local_time[64];

        strftime(local_time,
                 sizeof(local_time),
                 "%Y-%m-%d %H:%M:%S",
                 tm_info);

        printf("\n--- Клиент ---\n");
        printf("PID клиента: %d\n", getpid());
        printf("Время клиента: %s\n", local_time);
        printf("Получено из памяти:\n");
        printf("  Время сервера: %s\n", data->time_str);
        printf("  PID сервера: %d\n", data->sender_pid);
        printf("---------------\n");

        sleep(2);
    }

    return 0;
}
