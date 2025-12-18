#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <string.h>
#include <time.h>

#define SHM_NAME "/time_shm"
#define LOCK_FILE "/tmp/time_server.lock"

typedef struct {
    char time_str[64];
    pid_t sender_pid;
} shared_data;

static int lock_instance(void) {
    int fd = open(LOCK_FILE, O_RDWR | O_CREAT, 0644);
    if (fd == -1) {
        perror("open lock file");
        return -1;
    }

    struct flock fl = {
        .l_type = F_WRLCK,
        .l_whence = SEEK_SET,
        .l_start = 0,
        .l_len = 0
    };

    if (fcntl(fd, F_SETLK, &fl) == -1) {
        close(fd);
        return -1;
    }

    return fd;
}

int main(void) {
    int lock_fd = lock_instance();
    if (lock_fd == -1) {
        printf("Сервер уже запущен. Повторный запуск невозможен.\n");
        return 0;
    }

    int shm_fd = shm_open(SHM_NAME, O_CREAT | O_RDWR, 0666);
    if (shm_fd == -1) {
        perror("shm_open");
        return 1;
    }

    ftruncate(shm_fd, sizeof(shared_data));

    shared_data *data = mmap(NULL,
                             sizeof(shared_data),
                             PROT_READ | PROT_WRITE,
                             MAP_SHARED,
                             shm_fd,
                             0);
    if (data == MAP_FAILED) {
        perror("mmap");
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

    return 0;
}
