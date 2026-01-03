#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <unistd.h>
#include <time.h>

#define SHM_KEY 0x1234
#define SEM_KEY 0x5678

struct shared_data {
    char message[256];
};

void sem_lock(int semid) {
    struct sembuf sb = {0, -1, 0};
    semop(semid, &sb, 1);
}

void sem_unlock(int semid) {
    struct sembuf sb = {0, 1, 0};
    semop(semid, &sb, 1);
}

void get_time(char *buf, size_t size) {
    time_t now = time(NULL);
    struct tm *tm_info = localtime(&now);

    snprintf(buf, size, "%02d:%02d:%02d",
             tm_info->tm_hour,
             tm_info->tm_min,
             tm_info->tm_sec);
}

int main() {
    int shmid = shmget(SHM_KEY, sizeof(struct shared_data), 0666);
    struct shared_data *data = shmat(shmid, NULL, 0);

    int semid = semget(SEM_KEY, 1, 0666);

    char time_str[16];

    while (1) {
        get_time(time_str, sizeof(time_str));

        sem_lock(semid);

        printf("[%s] Приемник PID=%d\n", time_str, getpid());
        printf("           Получено: %s\n\n", data->message);

        sem_unlock(semid);

        sleep(3);
    }

    return 0;
}
