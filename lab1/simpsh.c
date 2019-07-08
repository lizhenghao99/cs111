/*
 * NAME:	Zhenghao Li
 * EMAIL: 	lizhenghao99@g.ucla.edu
 * ID: 		704971934
 */

#include<stdio.h>
#include<unistd.h>
#include<stdlib.h>
#include<string.h>
#include<getopt.h>
#include<fcntl.h>
#include<errno.h>
#include<signal.h>
#include<sys/types.h>
#include<sys/wait.h>
#include<fcntl.h>

struct pid_node
{
	pid_t pid;
	char** arguments;
};

int countArgs(int index, int argc, char* argv[], int print)
{
	index--;
	while (index < argc)
	{
		char* buf = argv[index];
		if (strstr(buf, "--") != NULL)
		{
			break;
		}
		if (print)
			printf("%s ", buf);
		index++;
	}
	if (print)
		printf("\n");
	return index;
}


void cleanup(int n)
{
	for (int i = 3; i < n; i++)
		{
			int rc;
			if (fcntl(i, F_GETFD) != -1)
			{
				rc = close(i);
			}
			if (rc == -1)
			{
				perror("Cleanup Error");
				exit(-1);
			}
		}
}

void catch_handler (int catch_signal)
{
	fprintf(stderr, "%d caught\n", catch_signal);
	exit(catch_signal);
}

void execute_command(int index, int argc, char* argv[], int n, int *rc,
						struct pid_node pid_list[], int *list_index)
{
	index--;

	int start = index + 3;

	int file_array[3];
	int count = 0; 	

	while (index < argc)
	{
		char *buf = argv[index];
		if (strstr(buf, "--") != NULL)
		{
			break;
		}
		if (count < 3)
		{
			file_array[count] = strtol(buf, NULL, 10) + 3;
			count++;
		}
		index++;
	}

	if (count != 3)
	{
		fprintf(stderr, "--command: Requires 3 files\n");
		*rc = 1;
		return;
	}

	for (int i = 0; i < 3; i++)
	{
		if (file_array[i] < 0 || file_array[i] >= n 
			|| fcntl(file_array[i], F_GETFD) == -1)
		{
			fprintf(stderr, "--command: Bad file descriptors\n");
			*rc = 1;
			return;
		}
	}


	int end = index;
	int length = end - start;

	if (length == 0)
	{
		fprintf(stderr, "--command: Requires command\n");
		*rc = 1;
		return;
	}

	char** exec_argv = malloc(sizeof(char*)*(length+1));
	for (int i = 0; i < length; i++)
	{
		exec_argv[i] = argv[start];
		start++;
	}
	exec_argv[length] = NULL;
	
	/* debug message */
	/*
*	printf("args: ");
*	for (int i = 0; i < length; i++)
*	{
*		printf("%s ", exec_argv[i]);
*	}
*	printf("\n");
	*/
	/* end */
	
	/* fork */
	pid_t pid = fork();
	if (pid == -1)
	{
		perror("--command");
		*rc = 1;
		return;
	}
	/* child */
	else if (pid == 0)
	{
		for (int i = 0; i < 3; i++)
		{
			int r = dup2(file_array[i],i);
			if (r == -1)
			{
				perror("dup");
			}
		}
		cleanup(n);
		execvp(exec_argv[0], exec_argv);
		perror("execvp");
		printf("This should not be printed\n");
	}
	/* parent */
	else
	{
		pid_list[*list_index].pid = pid;
		pid_list[*list_index].arguments = exec_argv;
		(*list_index)++;
	}
}


int main(int argc, char* argv[])
{
	int rc = 0;
	int status;
	int final_status = 0;
	int final_signal = 0;
	int option_index = 0;
	int num_fd = 3;

	/* flags */
	int open_flag = 0;
	int verbose_flag = 0;
	int signal_flag = 0;

	struct pid_node pid_list[32];
	int list_index = 0;

	struct option long_options[] = 
	{
		{"rdonly",	required_argument,	0,	0},
		{"wronly",	required_argument,	0,	1},
		{"rdwr",	required_argument,	0,	2},
		{"command",	required_argument,	0,	3},
		{"verbose",	no_argument,		0,	4},
		{"append",	no_argument,		0,	5},
		{"cloexec",	no_argument,		0,	6},
		{"creat", 	no_argument,		0,	7},
		{"directory",no_argument,		0,	8},
		{"dsync", 	no_argument,		0,	9},
		{"excl",	no_argument,		0,	10},
		{"nofollow",no_argument,		0,	11},
		{"nonblock",no_argument,		0,	12},
		{"rsync",	no_argument,		0,	13},
		{"sync",	no_argument,		0,	14},
		{"trunc",	no_argument,		0,	15},
		{"pipe",	no_argument,		0,	16},
		{"close",	required_argument,	0,	17},
		{"abort",	no_argument,		0,	18},
		{"catch",	required_argument,	0,	19},
		{"ignore",	required_argument,	0,	20},
		{"default",	required_argument,	0,	21},
		{"pause",	no_argument,		0,	22},
		{"wait",	no_argument,		0,	23},
		{0,			0,					0,	0}
	};

	/* option handle */
	int option = -1;
	int fd;
	while ((option = 
			getopt_long(argc, argv, "", 
			long_options, &option_index)) != -1)
	{	
		switch(option)
		{
			case 0:
				if (verbose_flag)
                    printf("--rdonly %s\n", optarg);
				if (open_flag & O_CREAT)
					fd = open(optarg, open_flag | O_RDONLY | O_CREAT, 0666);
				else
					fd = open(optarg, open_flag | O_RDONLY);
				if (fd == -1)
				{
					perror(optarg);
					rc = 1;
				}
				if (fd != -1)
					dup2(fd, num_fd);
				if (fd != num_fd)
					close(fd);
				num_fd++;
				open_flag = 0;
				break;
			case 1:
				if (verbose_flag)
                    printf("--wronly %s\n", optarg);
				if (open_flag & O_CREAT)
					fd = open(optarg, open_flag | O_WRONLY | O_CREAT, 0666);
				else
					fd = open(optarg, open_flag | O_WRONLY);
				if (fd == -1)
				{
					perror(optarg);
					rc = 1;
				}
				if (fd != -1)
					dup2(fd, num_fd);
				if (fd != num_fd)
					close(fd);
				num_fd++;
				open_flag = 0;
				break;
			case 2:
				if (verbose_flag)
                    printf("--rdwr %s\n", optarg);
				if (open_flag & O_CREAT)
					fd = open(optarg, open_flag | O_RDWR | O_CREAT, 0666);
				else
					fd = open(optarg, open_flag | O_RDWR);
				if (fd == -1)
				{
					perror(optarg);
					rc = 1;
				}
				if (fd != -1)
					dup2(fd, num_fd);
				if (fd != num_fd)
					close(fd);
				num_fd++;
				open_flag = 0;
				break;
			case 3:
				if (verbose_flag)
				{
					printf("--command ");
					countArgs(optind, argc, argv, 1);
				}
				fflush(stdout);
				execute_command(optind, argc, argv, num_fd, &rc, 
								pid_list, &list_index);
				optind = countArgs(optind, argc, argv, 0);
				break;
			case 4:
				verbose_flag = 1;
				break;
			case 5:
				if (verbose_flag)
					printf("--append\n");
				open_flag = open_flag | O_APPEND;
				break;
			case 6:
				if (verbose_flag)
					printf("--cloexec\n");
				open_flag = open_flag | O_CLOEXEC;
				break;
			case 7:
				if (verbose_flag)
					printf("--creat\n");
				open_flag = open_flag | O_CREAT;
				break;
			case 8:
				if (verbose_flag)
					printf("--directory\n");
				open_flag = open_flag | O_DIRECTORY;
				break;
			case 9:
				if (verbose_flag)
					printf("--dsync\n");
				open_flag = open_flag | O_DSYNC;
				break;
			case 10:
				if (verbose_flag)
					printf("--excl\n");
				open_flag = open_flag | O_EXCL;
				break;
			case 11:
				if (verbose_flag)
					printf("--nofollow\n");
				open_flag = open_flag | O_NOFOLLOW;
				break;
			case 12:
				if (verbose_flag)
					printf("--nonblock\n");
				open_flag = open_flag | O_NONBLOCK;
				break;
			case 13:
				if (verbose_flag)
					printf("--rsync\n");
				open_flag = open_flag | O_RSYNC;
				break;
			case 14:
				if (verbose_flag)
					printf("--sync\n");
				open_flag = open_flag | O_SYNC;
				break;
			case 15:
				if (verbose_flag)
					printf("--trunc\n");
				open_flag = open_flag | O_TRUNC;
				break;
			case 16:
				if (verbose_flag)
					printf("--pipe\n");
				if (1)
				{
					int pipefd[2];
					fd = pipe(pipefd);
					if (fd == -1)
					{
						perror("pipe");
						num_fd += 2;
					}
					else
					{
						dup2(pipefd[0], num_fd);
						num_fd++;
						dup2(pipefd[1], num_fd);
						num_fd++;
					}
				}
				break;
			case 17:
			{
				if (verbose_flag)
					printf("--close %s\n", optarg);
				int closefd = strtol(optarg, NULL, 10) + 3;
				int n = close(closefd);
				if (n == -1)
				{
					perror("close");
					rc = 1;		
				}
				break;
			}
			case 18:
			{
				if (verbose_flag)
					printf("--abort\n");
				fflush(stdout);
				int *ptr = NULL;
				int n = *ptr;
				return n;
				break;
			}
			case 19:
			{
				if (verbose_flag)
					printf("--catch %s\n", optarg);
				int catch_signal = strtol(optarg, NULL, 10);
				signal(catch_signal, catch_handler);
				break;
			}
			case 20:
			{
				if (verbose_flag)
					printf("--ignore %s\n", optarg);
				int ignore_signal = strtol(optarg, NULL, 10);
				signal(ignore_signal, SIG_IGN);
				break;
			}
			case 21:
			{
				if (verbose_flag)
					printf("--default %s\n", optarg);
				int default_signal = strtol(optarg, NULL, 10);
				signal(default_signal, SIG_DFL);
				break;
			}
			case 22:
				if (verbose_flag)
					printf("--pause\n");
				pause();
				break;
			case 23:
			{
				if (verbose_flag)
					printf("--wait\n");				
				pid_t waitrc;
				while ((waitrc = wait(&status)) != -1)
				{
					signal_flag = 0;
					int current_status;
					int current_signal;
					if (WIFSIGNALED(status))
					{
						signal_flag = 1;
						current_signal = WTERMSIG(status);
						if (current_signal > final_signal)
							final_signal = current_signal;
					}
					if (WIFEXITED(status))
					{
						current_status = WEXITSTATUS(status);
						if (current_status > final_status)
							final_status = current_status;
					}
					if (signal_flag)
						printf("signal %d ", current_signal);
					else
						printf("exit %d ", current_status);
					for (int i = 0; i < list_index; i++)
					{
						if (pid_list[i].pid == waitrc)
						{
							for (int j = 0; ; j++)
							{
								char* buf = pid_list[i].arguments[j];
								if (buf != NULL)
									printf("%s ", buf);
								else
								{
									printf("\n");
									break;
								}
							}
							break;
						}
					}
				}

			}
				break;
			default:
				rc = 1;
				break;
		}
	}

	/* debug message */
	/* printf("num_fd: %d rc: %d\n", num_fd, rc); */
	fflush(stdout);
	cleanup(num_fd);
	for (int i = 0; i < list_index; i++)
		free(pid_list[i].arguments);
	if (final_status == 0)
		final_status = rc;
	if (final_signal)
	{
		signal(final_signal, SIG_DFL);
		raise(final_signal);
	}
	else
		exit(final_status);
}
