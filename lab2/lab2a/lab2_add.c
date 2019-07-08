/*
 * NAME:	Zhenghao Li
 * EMAIL:	lizhenghao99@g.ucla.edu
 * ID:		704971934
 */

#include<stdlib.h>
#include<stdio.h>
#include<string.h>
#include<unistd.h>
#include<getopt.h>
#include<pthread.h>
#include<time.h>

static pthread_mutex_t lock_mutex = PTHREAD_MUTEX_INITIALIZER;
static int opt_yield = 0;

typedef struct __thread_args
{
	long long *pointer;
	int iterations;
} thread_args;

typedef struct __lock_t
{
	int flag;
} lock_t;

static lock_t lock;

void init(lock_t *lock)
{
	lock->flag = 0;
}

void s_lock(lock_t *lock)
{
	while (__sync_lock_test_and_set(&(lock->flag), 1) == 1)
		; // spin
}

void s_unlock(lock_t *lock)
{
	__sync_lock_release(&(lock->flag));
}

void add(long long *pointer, long long value)
{
	long long sum = *pointer + value;
	if (opt_yield)
		sched_yield();
	*pointer = sum;
}

void *add_none(void *args)
{
	thread_args *myargs = (thread_args*) args;
	long long *count = myargs->pointer;
	int num = myargs->iterations;
	for (int i = 0; i < num; i++)
		add(count, 1);
	for (int i = 0; i < num; i++)
		add(count, -1);
	return NULL;
}

void *add_m(void *args)
{
    thread_args *myargs = (thread_args*) args;
    long long *count = myargs->pointer;
    int num = myargs->iterations;
    for (int i = 0; i < num; i++)
	{
		int rc;
		rc = pthread_mutex_lock(&lock_mutex);
		if (rc) exit(1);
        add(count, 1);
		rc = pthread_mutex_unlock(&lock_mutex);
		if (rc) exit(1);
	}
    for (int i = 0; i < num; i++)
	{
		int rc;
		rc = pthread_mutex_lock(&lock_mutex);
		if (rc) exit(1);
        add(count, -1);
		rc = pthread_mutex_unlock(&lock_mutex);
		if (rc) exit(1);
	}
    return NULL;
}

void *add_s(void *args)
{
    thread_args *myargs = (thread_args*) args;
    long long *count = myargs->pointer;
    int num = myargs->iterations;

    for (int i = 0; i < num; i++)
	{
		s_lock(&lock);
        add(count, 1);
		s_unlock(&lock);
	}
    for (int i = 0; i < num; i++)
	{
		s_lock(&lock);	
        add(count, -1);
		s_unlock(&lock);
	}
    return NULL;
}

void *add_c(void *args)
{
    thread_args *myargs = (thread_args*) args;
    long long *count = myargs->pointer;
    int num = myargs->iterations;
    for (int i = 0; i < num; i++)
    {
		for (;;)
		{
			long long old = *count;
			long long new = old + 1;
			if (opt_yield)
				sched_yield();
			if (__sync_val_compare_and_swap(count, old, new) == old)
				break;
		}
	}
	for (int i = 0; i < num; i++)
    {
		for (;;)
		{
			long long old = *count;
			long long new = old - 1;
			if (opt_yield)
				sched_yield();
			if (__sync_val_compare_and_swap(count, old, new) == old)
				break;
		}
	}
	return NULL;
}

int main(int argc, char *argv[])
{

	int num_threads = 1;
	int num_iterats = 1;
	long long counter = 0;
	int rc = 0;
	void *(*thread_routine)(void*) = add_none;

	char tag[32] = "add";
	long long time_total = 0;
	long long time_avg = 0;
	struct timespec time_start;
	struct timespec time_end;

	/* option initialization */
	int option_index = 0;
	struct option long_options[] =
	{
		{"threads",		required_argument,	0,	0},
		{"iterations",	required_argument,	0,	1},
		{"yield",		no_argument,		0,	2},
		{"sync",		required_argument,	0,	3},
		{0,				0,					0,	0}
	};

	/* handle options */
	int option = -1;

	while ((option = 
			getopt_long(argc, argv, "", long_options, &option_index)) != -1)
	{
		switch(option)
		{
			case 0: 
				num_threads = strtol(optarg, NULL, 10);	
				break;
			case 1:
				num_iterats = strtol(optarg, NULL, 10);
				break;
			case 2:
				opt_yield = 1;
				break;
			case 3:
				if (strcmp(optarg, "m") == 0)
					thread_routine = add_m;
				else if (strcmp(optarg, "s") == 0)
				{
					thread_routine = add_s;
					init(&lock);
				}
				else if (strcmp(optarg, "c") == 0)
					thread_routine = add_c;
				else
				{
					fprintf(stderr, "Error: unrecognized option\n");
					exit(1);
				}
				break;
			default:
				fprintf(stderr, "Error: unrecognized option\n");
				exit(1);
		}
	}

	/* get start time */
	clock_gettime(CLOCK_MONOTONIC, &time_start);

	/* thread argument initialization */
	thread_args args;
	args.pointer = &counter;
	args.iterations = num_iterats;

	/* create threads */
	pthread_t* thread_array = (pthread_t*) 
								malloc(sizeof(pthread_t*)*num_threads);
	if (!thread_array)
	{
		fprintf(stderr, "malloc error!\n");
		exit(1);
	}
	for (int i = 0; i < num_threads; i++)
	{
		rc = pthread_create(&thread_array[i], NULL, thread_routine, &args);
		if (rc)
		{
			fprintf(stderr, "pthread_create error!\n");
			free(thread_array);
			exit(1);
		}
	}
	
	/* wait threads */
	for (int i = 0; i < num_threads; i++)
	{
		rc = pthread_join(thread_array[i], NULL);
		if (rc)
		{
			fprintf(stderr, "pthread_join error!\n");
			free(thread_array);
			exit(1);
		}
	}
	
	/* get end time */
	clock_gettime(CLOCK_MONOTONIC, &time_end);
	long long ssec = (long long) time_start.tv_sec;
	long long snsec = (long long) time_start.tv_nsec;
	long long esec = (long long) time_end.tv_sec;
	long long ensec = (long long) time_end.tv_nsec;
	time_total = (esec - ssec)*1e9 + (ensec - snsec);
	

	/* output */
	int num_ops = num_threads * num_iterats * 2;
	time_avg = time_total/num_ops;

	if (opt_yield)
		strcat(tag, "-yield");

	if (thread_routine == add_m)
		strcat(tag, "-m");
	else if (thread_routine == add_s)
		strcat(tag, "-s");
	else if (thread_routine == add_c)
		strcat(tag, "-c");
	else
		strcat(tag, "-none");

	fprintf(stdout, "%s,%d,%d,%d,%lld,%lld,%lld\n", tag, num_threads, 
					num_iterats, num_ops, time_total, time_avg, counter);	

	/* cleanup */
	free(thread_array);

	return 0;
}
