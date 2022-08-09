#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>
int count_server_connection_times = 0;

void Writing_string_File(char *my_str, int fd_srv)
{
	for (int i = 0; i < strlen(my_str); i++)
	{
		// printf("%d count ",i);

		if (write(fd_srv, &my_str[i], 1) < -1)
		{
			printf("Can't write into to_srv file. Please try again! \n");
			if (fork())
				wait(NULL);
			else
			{
				execlp("rm", "rm", "to_srv", NULL);
				printf("Can't Remove to_srv file \n");
				exit(-1);
			}
		}
		else
			continue;
	}
	if (write(fd_srv, "\n", 1) < -1)
	{
		printf("Can't write into to_srv file. Please try again! \n");
		if (fork())
			wait(NULL);
		else
		{
			execlp("rm", "rm", "to_srv", NULL);
			printf("Can't Remove to_srv file \n");
		}
		exit(-1);
	}
}

void pid_to_buff(char *my_buffer, pid_t my_pid, int inital_p)
{
	int i = 1, j = 0;
	int temp = my_pid;

	while (true)
	{
		temp = temp / 10;
		if (temp == 0)
		{
			break;
		}
		j++;
	}
	while (j != 0)
	{
		i *= 10;
		j--;
	}
	char temp_char;
	while (i != 0)
	{
		temp_char = (char)((my_pid / i) + 48);

		my_buffer[inital_p] = temp_char;

		my_pid = my_pid % i;

		inital_p++;
		// updating the value of i
		i = i / 10;
	}
	my_buffer[inital_p] = '\0';
}

void Signal_USR(int sig)
{
	char buffer[50];
	char to_client[] = "to_client_";
	int i = 0;
	int fd_to_client;
	char print_buffer;
	signal(SIGUSR1, Signal_USR);
	for (i = 0; i < strlen(to_client); i++)
		buffer[i] = to_client[i];
	pid_to_buff(buffer, getpid(), i);

	if (!(fd_to_client = open(buffer, O_RDONLY, 0666)) > 0)
	{
		printf("Cant open Client file. to_client_xxxxxx \n");
		exit(-1);
	}
	for (i = 1; i != 0;)
	{
		i = read(fd_to_client, &print_buffer, 1);
		if (i < 0)
		{
			printf("Cant remove to_client_xxxxx\n");
			if (fork())
				wait(NULL);
			else
			{
				execlp("rm", "rm", buffer, NULL);
				printf("Cant remove file to_client_xxxx\n");
			}
			exit(-1);
		}
		if (i > 0)
		{
			printf("%c", print_buffer);
		}
	}

	close(fd_to_client);

	if (!fork())
	{
		execlp("rm", "rm", buffer, NULL);
		printf("cant rempve file to_client_xxxx\n");
		exit(-1);
	}
	else
		wait(NULL);
	exit(1);
}

void my_alaram(int sig)
{
	signal(SIGALRM, my_alaram);
	count_server_connection_times = count_server_connection_times + 1;
}
void finish_alaram(int sig)
{
	exit(-1);
}

int main(int argc, char *argv[])
{

	int fd_srv;
	int random;
	char my_buffer[50];
	pid_t server_pid;

	signal(SIGUSR1, Signal_USR);
	signal(SIGALRM, my_alaram);

	if (argc != 5)
	{
		printf("There must be 5 arguments \n");
		exit(-1);
	}
	server_pid = atoi(argv[1]);
	int check_arg=atoi(argv[4]);
	if (check_arg==0)
	{
		printf("cant divide value by zero!\n");
		exit(-1);
	}
	if ((kill(server_pid, 0)) < 0)
	{
		printf("Not valid PID Value \n");
		exit(-1);
	}
	while (count_server_connection_times < 10)
	{
		fd_srv = open("to_srv", O_WRONLY | O_CREAT | O_EXCL, 0666);
		if (fd_srv < 0)
		{
			if (errno == EEXIST)
			{
				random = (rand() % 10) + 1;
				alarm(random);
				sleep(random);
			}
			else
			{
				printf("Cant create to_srv file\n");
				exit(-1);
			}
		}
		else
			break;
	}
	// check for 10 times connection
	if (count_server_connection_times == 10)
	{
		unsigned int trigger = 30000;
		unsigned int elapsed_seconds = 0;
		clock_t start = clock();
		do
		{
			clock_t difference = clock() - start;
			elapsed_seconds = difference * 1000 / CLOCKS_PER_SEC;
		} while (elapsed_seconds < trigger);
		printf("Client closed because no response was received from the server for 30 seconds \n");
		exit(-1);
	}

	sprintf(my_buffer, "%d", getpid());
	Writing_string_File(my_buffer, fd_srv);
	Writing_string_File(argv[2], fd_srv);
	Writing_string_File(argv[3], fd_srv);
	Writing_string_File(argv[4], fd_srv);
	if ((kill(server_pid, SIGUSR1)) < 0)
	{
		printf("cant send signal to the server \n");
		if (!fork())
		{
			execlp("rm", "rm", "to_srv", NULL);
			printf("cant remove the srv file");
			exit(-1);
		}
		else
		{

			wait(NULL);
		}
		exit(-1);
	}

	signal(SIGALRM, finish_alaram);
	alarm(20);
	pause();
	exit(1);
}
