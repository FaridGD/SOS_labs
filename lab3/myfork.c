#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>

void exit_handler(void)
{
	printf("Процесс %d завершает работу через atexit()\n", getpid());
}

void sigint_handler(int sig)
{
	printf("Процесс %d получил сигнал SIGINT (%d)\n", getpid(), sig);
}

void sigterm_handler(int sig)
{
	printf("Процесс %d получил сигнал SIGTERM (%d)\n", getpid(), sig);
}

int main()
{
	pid_t pid;
	int status;
	struct sigaction sa;
	atexit(exit_handler);
	signal(SIGINT, sigint_handler);
	sa.sa_handler = sigterm_handler;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = 0;
	sigaction(SIGTERM, &sa, NULL);
	pid = fork();
	if (pid < 0)
	{
		perror("fork error");
		exit(EXIT_FAILURE);
	}
	else if (pid == 0)
	{
		printf("Дочерний процесс: PID = %d, PPID = %d\n", getpid(), getppid());
		sleep(2);
		exit(5);
	}
	else
	{
		printf("Родительский процесс: PID = %d, PPID = %d\n", getpid(), getppid());
		waitpid(pid, &status, 0);
		if (WIFEXITED(status))
		{
			printf("Дочерний процесс завершился с кодом %d\n", WEXITSTATUS(status));
		}
	}
	return 0;
}

