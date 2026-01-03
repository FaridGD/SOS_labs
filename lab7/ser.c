#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <time.h>
#include <string.h>

#define SHM_KEY 0x1234

typedef struct {
    char time_str[64];
    pid_t sender_pid;
} shared_data;

int main(void) {
    int shmid = shmget(SHM_KEY,
                       sizeof(shared_data),
                       IPC_CREAT | IPC_EXCL | 0666);

    if (shmid == -1) {
        printf("Сервер уже запущен. Повторный запуск невозможен.\n");
        return 0;
    }

    shared_data *data = (shared_data *)shmat(shmid, NULL, 0);
    if (data == (void *)-1) {
        perror("shmat");
        return 1;
    }

    printf("Сервер запущен (PID: %d)\n", getpid());

    while (1) {
        time_t now = time(NULL);
        struct tm *tm_info = localtime(&now);

        strftime(data->time_str,
                 sizeof(data->time_str),
                 "%Y-%m-%d %H:%M:%S",
                 tm_info);

        data->sender_pid = getpid();

        sleep(1);
    }

    shmdt(data);
    shmctl(shmid, IPC_RMID, NULL);
    return 0;
}
