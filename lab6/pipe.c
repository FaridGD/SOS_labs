#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include <errno.h>
#include <stdbool.h>

#define PIPE_INPUT 0
#define PIPE_OUTPUT 1
#define MAX_BUFFER 1024

int main() {
    int channel[2];
    
    if (pipe(channel) != 0) {
        perror("Ошибка создания канала");
        exit(EXIT_FAILURE);
    }
    
    pid_t child = fork();
    
    if (child < 0) {
        perror("Ошибка создания процесса");
        exit(EXIT_FAILURE);
    }
    
    if (child == 0) {
        close(channel[PIPE_OUTPUT]);
        

        printf("Дочерний процесс ожидает данные...\n");
        sleep(10);
        
        char message[MAX_BUFFER];
        ssize_t bytes;
        
        bytes = read(channel[PIPE_INPUT], message, MAX_BUFFER - 1);
        
        if (bytes < 0) {
            perror("Ошибка чтения");
            close(channel[PIPE_INPUT]);
            exit(EXIT_FAILURE);
        }
        
        message[bytes] = '\0';
        
        printf("[Канал] Получено: %s\n", message);

        time_t now = time(NULL);
        struct tm *local = localtime(&now);
        char time_buf[64];
        strftime(time_buf, sizeof(time_buf), "%H:%M:%S", local);
        
        printf("PID процесса: %d. Текущее время: %s\n", getpid(), time_buf);
        
        close(channel[PIPE_INPUT]);
        exit(EXIT_SUCCESS);
        
    } else {
        close(channel[PIPE_INPUT]);
        
        time_t current = time(NULL);
        struct tm *timeinfo = localtime(&current);
        char timestamp[64];
        strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", timeinfo);
        
        char full_message[MAX_BUFFER];
        int length = snprintf(full_message, MAX_BUFFER,
                             "Привет от родительского процесса %d. Время отправки: %s",
                             getpid(), timestamp);
        
        if (length < 0) {
            perror("Ошибка форматирования");
            close(channel[PIPE_OUTPUT]);
            exit(EXIT_FAILURE);
        }
        
        ssize_t written = write(channel[PIPE_OUTPUT], full_message, length);
        
        if (written < 0) {
            perror("Ошибка записи");
            close(channel[PIPE_OUTPUT]);
            exit(EXIT_FAILURE);
        }
        
        printf("Сообщение отправлено дочернему процессу\n");
        
        int status;
        waitpid(child, &status, 0);
        
        if (WIFEXITED(status)) {
            printf("Дочерний процесс завершился с кодом %d\n", WEXITSTATUS(status));
        }
        
        close(channel[PIPE_OUTPUT]);
    }
    
    return EXIT_SUCCESS;
}