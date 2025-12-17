#include "fifo.h"

#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <time.h>

int main(void)
{
    char local_buf[FIFO_BUF_CAPACITY];

    int fd = open(FIFO_FILE, O_RDWR);
    if (fd < 0) {
        fprintf(stderr, "[reader] open failed: %s\n", strerror(errno));
        return 1;
    }

    ssize_t bytes = read(fd, local_buf, sizeof(local_buf) - 1);
    if (bytes < 0) {
        fprintf(stderr, "[reader] read failed: %s\n", strerror(errno));
        close(fd);
        return 1;
    }

    local_buf[bytes] = '\0';
    printf("[fifo message] %s\n", local_buf);

    time_t now = time(NULL);
    printf("Reader PID: %d\nTime: %s",
           getpid(),
           ctime(&now));

    close(fd);
    return 0;
}
