/*
 * NAME:	Zhenghao Li
 * EMAIL:	lizhenghao99@g.ucla.edu
 * ID:		704971934
 */

#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <poll.h>
#include <signal.h>
#include <mraa.h>
#include <math.h>
#include <pthread.h>
#include <time.h>
#include <sys/time.h>
#include <errno.h>
#include <getopt.h>

mraa_aio_context temp_sensor;
mraa_gpio_context button;
int fd = -2;
int exit_code = 0;
volatile int flag_start = 1;
volatile int flag_off = 0;
volatile int flag_log = 0;
volatile int period = 1;
volatile char scale = 'F';

void print_time()
{
	struct tm *local;
	struct timeval clock;

	gettimeofday(&clock, NULL);
	local = localtime(&clock.tv_sec);
	printf("%02d:%02d:%02d ", local->tm_hour, local->tm_min, local->tm_sec);
	if (flag_log)
		dprintf(fd, "%02d:%02d:%02d ", local->tm_hour, local->tm_min, local->tm_sec);
}

void process_command(char* command)
{
	char* periodptr = strstr(command, "PERIOD=");
	char* scaleptr = strstr(command, "SCALE=");
	char buf[128];
	
	if (flag_log)
		dprintf(fd, "%s", command);

	if (strcmp(command, "STOP\n") == 0)
		flag_start = 0;

	else if (strcmp(command, "START\n") == 0)
		flag_start = 1;

	else if (strcmp(command, "OFF\n") == 0)
	{
		print_time();
		printf("SHUTDOWN\n");
		if (flag_log)
			dprintf(fd, "SHUTDOWN\n");
		flag_off = 1;
	}

	else if (strstr(command, "LOG") == command)
	{
		printf("%s", command);
	}

	else if (periodptr != NULL)
	{
		int i;
		for (i = 0; periodptr[i+7] != '\n'; i++)
			buf[i] = periodptr[i+7];
		if (i == 0)
		{
			fprintf(stderr, "ERROR: no argument for PERIOD\n");
			exit_code = 1;
		}
		else
		{
			buf[i] = '\0';
			period = strtol(buf, NULL, 10);
		}
	}

	else if (scaleptr != NULL)
	{
		if (scaleptr[6] == 'F'|| scaleptr[6] == 'C')
			scale = scaleptr[6];
		else
		{
			fprintf(stderr, "ERROR: invalid argument for SCALE\n");
			exit_code = 1;
		}
	}

	else
	{
		fprintf(stderr, "ERROR: invalid command\n");
		exit_code = 1;
	}
}

float get_temp(int read)
{
	int B = 4275;
	int R0 = 100000;

	float R = R0*(1023.0/read-1.0);
	float temp = 1.0/(log(R/R0)/B+1/298.15)-273.15;
	return temp;
}

void *report_temp()
{
	temp_sensor = mraa_aio_init(1);
	if (temp_sensor == NULL)
	{
		fprintf(stderr, "Error: failure to initialize temperature sensor\n");
		mraa_deinit();
		mraa_gpio_close(button);
		exit(1);
	}

	time_t prev = 0;
	while (1)
	{
		struct timeval clock;
		gettimeofday(&clock, NULL);
		time_t curr = clock.tv_sec;

		if (flag_start && curr-prev >= period)
		{
			int read = mraa_aio_read(temp_sensor);
			float temp = get_temp(read);
			if (scale == 'F')
				temp = temp*9/5+32;
			else if (scale == 'C')
				;
			else
			{
				fprintf(stderr, "Error: scale is messed up. Debug\n");
				exit(1);
			}
			print_time();
			printf("%.1f\n", temp);
			if (flag_log)
				dprintf(fd, "%.1f\n", temp);
			prev = curr;
		}
		else if (!flag_start)
			prev = 0;
	}

	return NULL;
}

void int_handler()
{
	print_time();
	printf("SHUTDOWN\n");
	if (flag_log)
		dprintf(fd, "SHUTDOWN\n");
	flag_off = 1;
}

int main(int argc, char* argv[])
{	
	char command[128];
	struct pollfd pollfile;
	pollfile.fd = STDIN_FILENO;
	pollfile.events = POLLIN;
	
	int option_index = 0;
	
	struct option long_options[] = 
	{
		{"period",	required_argument,	0,	1}, 
		{"scale",	required_argument,	0,	2},
		{"log",		required_argument,	0,	3},
		{0,			0,					0,	0}
	};

	int option = -1;
	char *logpath = NULL;
	
	while ((option = getopt_long(argc, argv, "", long_options, &option_index))
				!= -1)
	{
		switch (option)
		{
			case 1:
				period = strtol(optarg, NULL, 10);
				break;
			case 2:
				if (strcmp(optarg, "C") == 0)
					scale = 'C';
				else if (strcmp(optarg, "F") == 0)
					scale = 'F';
				else
				{
					fprintf(stderr, "ERROR: invalid argument for --scale");
					exit(1);
				}
				break;
			case 3:
				flag_log = 1;
				logpath = optarg;
				break;
			default:
				fprintf(stderr, "ERROR: invalid option\n");
				exit(1);
		}
	}

	if (flag_log)
	{
		fd = open(logpath, O_CREAT|O_WRONLY|O_TRUNC, 0644);
		if (fd == -1)
		{
			perror("Open");
			exit(1);
		}
	}

	button = mraa_gpio_init(60);
	if (button == NULL)
	{
		fprintf(stderr, "Error: failure to initialize button\n");
		mraa_deinit();
		exit(1);
	}
	mraa_gpio_dir(button, MRAA_GPIO_IN);
	mraa_gpio_isr(button, MRAA_GPIO_EDGE_RISING, &int_handler, NULL);

	pthread_t tid;
	int rc = pthread_create(&tid, NULL, report_temp, NULL);
	if (rc)
	{
		fprintf(stderr, "Error: failure to create thread\n");
		exit(1);
	}

	while (!flag_off)
	{
		int rc = poll(&pollfile, 1, 0);
		if (rc > 0)
		{
			fgets(command, 128, stdin);	
			process_command(command);
		}
		else
			continue;
	}

	mraa_aio_close(temp_sensor);
	mraa_gpio_close(button);
	close(fd);
	return exit_code;
}
