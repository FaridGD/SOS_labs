#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <time.h>

#define SHM_KEY 0x1234

typedef struct {
    char time_str[64];
    pid_t sender_pid;
} shared_data;

int main(void) {
    printf("Клиент запущен (PID: %d)\n", getpid());

    int shmid = shmget(SHM_KEY, sizeof(shared_data), 0666);
    if (shmid == -1) {
        perror("shmget");
        return 1;
    }

    shared_data *data = (shared_data *)shmat(shmid, NULL, SHM_RDONLY);
    if (data == (void *)-1) {
        perror("shmat");
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
        printf("Получено:\n");
        printf("  Время сервера: %s\n", data->time_str);
        printf("  PID сервера: %d\n", data->sender_pid);
        printf("---------------\n");

        sleep(2);
    }

    shmdt(data);
    return 0;
}
