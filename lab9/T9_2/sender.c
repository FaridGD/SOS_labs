#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <unistd.h>
#include <time.h>
#include <string.h>

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
    int old_shmid = shmget(SHM_KEY, 0, 0);
    if (old_shmid != -1) {
        shmctl(old_shmid, IPC_RMID, NULL);
    }
    
    int old_semid = semget(SEM_KEY, 0, 0);
    if (old_semid != -1) {
        semctl(old_semid, 0, IPC_RMID);
    }
    
    int shmid = shmget(SHM_KEY, sizeof(struct shared_data), IPC_CREAT | 0666);
    struct shared_data *data = shmat(shmid, NULL, 0);
    
    strcpy(data->message, "Инициализация");
    
    int semid = semget(SEM_KEY, 1, IPC_CREAT | 0666);
    semctl(semid, 0, SETVAL, 1);

    char time_str[16];

    while (1) {
        get_time(time_str, sizeof(time_str));

        sem_lock(semid);
        snprintf(data->message, sizeof(data->message),
                 "Отправитель PID=%d, время=%s",
                 getpid(), time_str);
        sem_unlock(semid);

        sleep(3);
    }

    return 0;
}