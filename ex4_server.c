#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <errno.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>

void calculate(int my_signal)
{
	char my_buffer[50];
	char to_client[] = "to_client_";
	char *my_char_pointer;
	int i = 1;
	int my_counter = 0;
	int my_line_input = 1;
	int fd_srv;
	int fd_client;

	int first_operand, op_to_perform, Scnd_operand;
	float final_result;
	pid_t client_pid;
	signal(SIGUSR1, calculate);
	if (!fork())
	{
		if ((fd_srv = open("to_srv", O_RDONLY, 0666)) < 0)
		{
			printf("Can't open to_srv file\n");
			exit(-2);
		}

		for (; i != 0;)
		{
			if ((i = read(fd_srv, &my_buffer[my_counter], 1)) < 0)
			{
				printf("Error while reading to_srv file\n");
				exit(-2);
			}
			if (my_buffer[my_counter] == '\n')
			{
				my_buffer[my_counter] = '\0';
				if (my_line_input == 1)
				{
					my_line_input = 2;
					client_pid = atoi(my_buffer);
					first_operand = my_counter + 1;
				}

				else if (my_line_input == 2)
				{
					my_line_input = 3;
					first_operand = atoi(&my_buffer[first_operand]);
					op_to_perform = my_counter + 1;
				}
				else if (my_line_input == 3)
				{
					my_line_input = 4;
					op_to_perform = atoi(&my_buffer[op_to_perform]);
					Scnd_operand = my_counter + 1;
				}
				else if (my_line_input == 4)
				{
					i = 0;
					Scnd_operand = atoi(&my_buffer[Scnd_operand]);
				}
				else
					break;
			}

			my_counter++;
		}

		if (!fork())
		{
			execlp("rm", "rm", "to_srv", NULL);
			printf("Cant remove file");
			exit(-1);
		}
		else
		{

			wait(NULL);
		}
		if (op_to_perform == 1)
			final_result = first_operand + Scnd_operand;
		else if (op_to_perform == 2)
			final_result = first_operand - Scnd_operand;
		else if (op_to_perform == 3)
			final_result = first_operand * Scnd_operand;
		else if (op_to_perform == 4)
		{
			if (Scnd_operand)
				final_result = ((float)first_operand / Scnd_operand);
			else
			{
				printf("cant divide value with zero\n");
				exit(-1);
			}
		}
		else
		{
			printf("invalid operation must be between 1-4!\n");
			exit(-1);
		}

		for (i = 0; i < strlen(to_client); i++)
			my_buffer[i] = to_client[i];

		child_pid_to_buff(my_buffer, client_pid, i);

		if ((fd_client = open(my_buffer, O_RDWR | O_CREAT | O_APPEND, 0666)) < 0)
		{
			printf("cant create client file...\n");
			exit(-1);
		}
		my_char_pointer = realpath(my_buffer, NULL);
		if (my_char_pointer == NULL)
		{
			perror("Real path issue\n");
			exit(-1);
		}

		sprintf(my_buffer, "%.1f", final_result);
		write_into_file(my_buffer, fd_client);

		if ((kill(client_pid, SIGUSR1)) < 0)
		{
			printf("Cant connnect to client\n");
			if (!fork())
				wait(NULL);
			else
			{
				execlp("rm", "rm", my_char_pointer, NULL);
				printf("cant remove client file");
			}
			exit(-1);
		}
		free(my_char_pointer);
		exit(1);
	}
}

void write_into_file(char *my_string, int fd_srv)
{
	for (int i = 0; i < strlen(my_string); i++)
	{

		if (write(fd_srv, &my_string[i], 1) < -1)
		{
			printf("cant write into srv file\n");
			exit(-1);
		}
		else
			continue;
	}
	if (write(fd_srv, "\n", 1) < -1)
	{
		printf("cant write into srv file\n");
		exit(-1);
	}
}
void my_child_exist(int sig)
{
	pid_t pid;
	int status;
	signal(SIGCHLD, my_child_exist);
	while (true)
	{
		pid = waitpid(-1, &status, WNOHANG);
		if (pid <= 0 && errno == ECHILD)
		{
			break;
		}
		if (WIFEXITED(status) && WEXITSTATUS(status) == 254)
		{

			if (fork())
				wait(NULL);
			else
			{
				execlp("rm", "rm", "to_srv", NULL);
				printf("Error while removing to_srv file");
				exit(-1);
			}
			if (WEXITSTATUS(status) == 253)
				exit(-1);
		}
	}
}

void child_pid_to_buff(char *my_buffer, pid_t client_pid, int inital_point)
{

	int i = 1, j = 0, temp = client_pid;
	char c;

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
		i = i * 10;
		j--;
	}
	while (i != 0)
	{
		c = (char)((client_pid / i) + 48);
		my_buffer[inital_point] = c;
		inital_point++;
		client_pid = client_pid % i;
		i = i / 10;
	}
	my_buffer[inital_point] = '\0';
}

int main()
{
	int fd_srv;

	signal(SIGUSR1, calculate);
	signal(SIGCHLD, my_child_exist);

	fd_srv = open("to_srv", O_RDWR | O_APPEND, 0666);
	if (fd_srv > 0)
	{
		if (fork())
		{
			wait(NULL);
		}
		else
		{
			execlp("rm", "rm", "to_srv", NULL);
			printf("cant remove to_srv file\n");
			exit(-3);
		}
	}
	else if (fd_srv < 0 && errno != ENOENT)
	{

		printf("Error with to_srv file\n");
		exit(-1);
	}
	while (true)
	{
		unsigned int trigger = 60000;
		unsigned int elapsed_seconds = 0;
		clock_t start = clock();
		do
		{
			clock_t difference = clock() - start;
			elapsed_seconds = difference * 1000 / CLOCKS_PER_SEC;
		} while (elapsed_seconds < trigger);
		printf("The server was closed because no service request was received for the last 60 seconds \n");
		break;
	}

	exit(1);
}
