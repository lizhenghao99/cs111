/* 
 * NAME:	Zhenghao Li
 * EMAIL:	lizhenghao99@g.ucla.edu
 * ID:		704971934
 */

#include<SortedList.h>
#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<string.h>
#include<getopt.h>
#include<pthread.h>
#include<time.h>
#include<signal.h>

int opt_yield = 0;
static char chars[128] = {"0123456789abcdefghifklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ"};
static SortedList_t *list;
static SortedListElement_t *element_array;
static char **key_array;
static int num_threads = 1;
static int num_iterats = 1;

static pthread_mutex_t lock_mutex = PTHREAD_MUTEX_INITIALIZER;

typedef struct __lock_t
{
    int flag;
} lock_t;

static lock_t lock_spin;

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

void *list_none(void *args)
{
	int rc;
	int id = *(int*)args;
	int start = id*num_iterats;
	int end = (id+1) * num_iterats;
	for (int i = start; i < end; i++)
	{
		SortedList_insert(list, &element_array[i]);
	}
	int length = SortedList_length(list);
	if (length == -1)
	{
		fprintf(stderr, "Error: Corrupted list\n");
		exit(2);
	}
	for (int i = start; i < end; i++)
	{
		SortedListElement_t *ptr;
		ptr = SortedList_lookup(list, element_array[i].key);
		rc = SortedList_delete(ptr);
		if (rc)
		{
			fprintf(stderr, "Error: Corrupted list\n");
			exit(2);
		}
	}
	return NULL;
}

void *list_m(void *args)
{
	int rc;
	int id = *(int*)args;
	int start = id*num_iterats;
	int end = (id+1) * num_iterats;
	for (int i = start; i < end; i++)
	{
		rc = pthread_mutex_lock(&lock_mutex);
		if (rc) {exit(1);}
		SortedList_insert(list, &element_array[i]);
		rc = pthread_mutex_unlock(&lock_mutex);
		if (rc) {exit(1);}
	}
	rc = pthread_mutex_lock(&lock_mutex);
    if (rc) {exit(1);}
	int length = SortedList_length(list);
	rc = pthread_mutex_unlock(&lock_mutex);
    if (rc) {exit(1);}
	if (length == -1)
	{
		fprintf(stderr, "Error: Corrupted list\n");
		exit(2);
	}
	for (int i = start; i < end; i++)
	{
		SortedListElement_t *ptr;
		rc = pthread_mutex_lock(&lock_mutex);
        if (rc) {exit(1);}
		ptr = SortedList_lookup(list, element_array[i].key);
		rc = SortedList_delete(ptr);
		if (rc)
		{
			fprintf(stderr, "Error: Corrupted list\n");
			exit(2);
		}
		rc = pthread_mutex_unlock(&lock_mutex);
        if (rc) {exit(1);}
	}
	return NULL;
}

void *list_s(void *args)
{
    int rc;
    int id = *(int*)args;
    int start = id*num_iterats;
    int end = (id+1) * num_iterats;
    for (int i = start; i < end; i++)
    {
        s_lock(&lock_spin);
        SortedList_insert(list, &element_array[i]);
        s_unlock(&lock_spin);
    }
    s_lock(&lock_spin);
    int length = SortedList_length(list);
    s_unlock(&lock_spin);
    if (length == -1)
    {
        fprintf(stderr, "Error: Corrupted list\n");
        exit(2);
    }
    for (int i = start; i < end; i++)
    {
        SortedListElement_t *ptr;
        s_lock(&lock_spin);
        ptr = SortedList_lookup(list, element_array[i].key);
        rc = SortedList_delete(ptr);
        if (rc)
        {
            fprintf(stderr, "Error: Corrupted list\n");
            exit(2);
        }
        s_unlock(&lock_spin);
    }
    return NULL;
}

void sighandler()
{
	fprintf(stderr, "Error: segmentation fault caught\n");
	free(element_array);
	free(key_array);
	exit(2);
}

int main(int argc, char* argv[])
{
	int rc = 0;
	void *(*thread_routine)(void*) = list_none;
	
	int num_list = 1;
	char tag[32] = "list";
	long long time_total = 0;
    long long time_avg = 0;
    struct timespec time_start;
    struct timespec time_end;
	
	signal(SIGSEGV, sighandler);

	/* option initialization */
    int option_index = 0;
    struct option long_options[] =
    {
        {"threads",     required_argument,  0,  0},
        {"iterations",  required_argument,  0,  1},
        {"yield",       required_argument,  0,  2},
        {"sync",        required_argument,  0,  3},
        {0,             0,                  0,  0}
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
				if (strchr(optarg, 'i') != NULL)
					opt_yield = opt_yield | INSERT_YIELD;
				if (strchr(optarg, 'd') != NULL)
					opt_yield = opt_yield | DELETE_YIELD;
				if (strchr(optarg, 'l') != NULL)
					opt_yield = opt_yield | LOOKUP_YIELD;
				if (strchr(optarg, 'i') == NULL &&
					strchr(optarg, 'd') == NULL &&
					strchr(optarg, 'l') == NULL)
				{
					fprintf(stderr, "--yield: Invalid argument\n");
					exit(1);
				}
				break;
			case 3:
                if (strcmp(optarg, "m") == 0)
                    thread_routine = list_m;
                else if (strcmp(optarg, "s") == 0)
                {
                    thread_routine = list_s;
                    init(&lock_spin);
                }
                else
                {
                    fprintf(stderr, "Error: unrecognized option\n");
                    exit(1);
                }
                break;
			default:
				fprintf(stderr, "Error: Unrecognized option\n");	
				exit(1);
		}
	}

	/* initialize list */
	SortedListElement_t head;
	head.key = NULL;
	head.next = &head;
	head.prev = &head;
	list = &head;

	/* create elements */
	int num_elements = num_threads * num_iterats;
	/* malloc elements */
	element_array = (SortedListElement_t*)
				malloc(sizeof(SortedListElement_t)*num_elements);
	if (element_array == NULL)
	{
		fprintf(stderr, "Error: malloc for element_array failed\n");
		exit(1);
	}
	/* malloc keys */
	key_array = (char**) malloc(sizeof(char*)*num_elements);
	if (key_array == NULL)
	{
		fprintf(stderr, "Error: malloc for key_array failed\n");
		exit(1);
	}
	/* initialize elements */
	for (int i = 0; i < num_elements; i++)
	{
		key_array[i] = (char*) malloc(sizeof(char)*256);
		if (key_array[i] == NULL)
		{
			fprintf(stderr, "Error: malloc for key failed\n");
			exit(1);
		}
		for (int j = 0; j < 255; j++)
		{
			int index = rand()%62;
			key_array[i][j] = chars[index];
		}
		key_array[i][256] = '\0';
		element_array[i].key = key_array[i];
	}	

	/* create threads */
    pthread_t* thread_array = (pthread_t*)
                                malloc(sizeof(pthread_t*)*num_threads);
    if (!thread_array)
    {
        fprintf(stderr, "Error: Malloc for thread_array failed\n");
        exit(1);
    }

	/* create thread argument */
	int *ids = (int*) malloc(sizeof(int)*num_threads);
	if (ids == NULL)
	{
		fprintf(stderr, "Error: Malloc for ids failed\n");
		exit(1);
	}
	for (int i = 0; i < num_threads; i++)
	{
		ids[i] = i;
	}

	/* get start time */
    clock_gettime(CLOCK_MONOTONIC, &time_start);

    for (int i = 0; i < num_threads; i++)
    {
        rc = pthread_create(&thread_array[i], NULL, thread_routine, 
													(void*)&ids[i]);
        if (rc)
        {
            fprintf(stderr, "Error: pthread_create failed\n");
            free(thread_array);
			free(element_array);
			free(key_array);
			free(ids);
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
	
	/* check length */
	int length = SortedList_length(list);
	if (length != 0)
	{
		fprintf(stderr, "Error: non-zero failure\n");
		free(thread_array);
		free(element_array);
		free(key_array);
		free(ids);
		exit(2);
	}	

	/* output */
	/* output */
    int num_ops = num_threads * num_iterats * 3;
    time_avg = time_total/num_ops;

    if (opt_yield)
		{
			strcat(tag, "-");
			if (opt_yield & INSERT_YIELD)
				strcat(tag, "i");
			if (opt_yield & DELETE_YIELD)
				strcat(tag, "d");
			if (opt_yield & LOOKUP_YIELD)
				strcat(tag, "l");
		}
	else
        strcat(tag, "-none");

	if (thread_routine == list_m)
		strcat(tag, "-m");
	else if (thread_routine == list_s)
		strcat(tag, "-s");
	else
        strcat(tag, "-none");
	
    fprintf(stdout, "%s,%d,%d,%d,%d,%lld,%lld\n", tag, num_threads,
                    num_iterats, num_list, num_ops, time_total, time_avg);

	free(thread_array);
	free(element_array);
	free(key_array);
	free(ids);
	return 0;
}
