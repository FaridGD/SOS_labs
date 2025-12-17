#include "fifo.h"

#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>
#include <time.h>

static int create_fifo_if_needed(void)
{
    unlink(FIFO_FILE);

    if (mkfifo(FIFO_FILE, 0666) < 0) {
        if (errno != EEXIST) {
            fprintf(stderr, "[writer] mkfifo failed: %s\n", strerror(errno));
            return -1;
        }
    }
    return 0;
}

int main(void)
{
    char out_buf[FIFO_BUF_CAPACITY];

    if (create_fifo_if_needed() != 0) {
        return 1;
    }

    int fd = open(FIFO_FILE, O_RDWR);
    if (fd < 0) {
        fprintf(stderr, "[writer] open failed: %s\n", strerror(errno));
        return 1;
    }

    time_t now = time(NULL);
    int len = snprintf(out_buf,
                       sizeof(out_buf),
                       "process %d. Current time is %s",
                       getpid(),
                       ctime(&now));

    if (len < 0) {
        fprintf(stderr, "[writer] snprintf failed\n");
        close(fd);
        return 1;
    }

    if (write(fd, out_buf, len) < 0) {
        fprintf(stderr, "[writer] write failed: %s\n", strerror(errno));
        close(fd);
        return 1;
    }

    printf("Message successfully written to FIFO\n");
    printf("Writer is waiting before exit (15 seconds)...\n");

    sleep(15);

    close(fd);
    return 0;
}
