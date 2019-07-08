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
static SortedListElement_t *element_array;
static char **key_array;
static int num_threads = 1;
static int num_iterats = 1;
static int num_lists = 1;

typedef struct __lock_t
{
    int flag;
} lock_t;

typedef struct __SubList_t
{
	SortedList_t *list;
	pthread_mutex_t lock_mutex;
	lock_t lock_spin;
} SubList_t;

static SubList_t *lists;

int Pthread_mutex_lock(pthread_mutex_t *lock, long long *total)
{
	int rc = 0;
    struct timespec time_start;
    struct timespec time_end;
    long long time_lock_single = 0;
    clock_gettime(CLOCK_MONOTONIC, &time_start);
    rc = pthread_mutex_lock(lock);
    clock_gettime(CLOCK_MONOTONIC, &time_end);
    long long ssec = (long long) time_start.tv_sec;
    long long snsec = (long long) time_start.tv_nsec;
    long long esec = (long long) time_end.tv_sec;
    long long ensec = (long long) time_end.tv_nsec;
    time_lock_single = (esec - ssec)*1e9 + (ensec - snsec);;
    *total += time_lock_single;
	return rc;
}

void s_init(lock_t *lock)
{
    lock->flag = 0;
}

void s_lock(lock_t *lock, long long *total)
{
	struct timespec time_start;
    struct timespec time_end;
    long long time_lock_single = 0;
	clock_gettime(CLOCK_MONOTONIC, &time_start);
    while (__sync_lock_test_and_set(&(lock->flag), 1) == 1)
        ; // spin
	clock_gettime(CLOCK_MONOTONIC, &time_end);
	long long ssec = (long long) time_start.tv_sec;
    long long snsec = (long long) time_start.tv_nsec;
    long long esec = (long long) time_end.tv_sec;
    long long ensec = (long long) time_end.tv_nsec;
    time_lock_single = (esec - ssec)*1e9 + (ensec - snsec);;
	*total += time_lock_single;
}

void s_unlock(lock_t *lock)
{
    __sync_lock_release(&(lock->flag));
}

int hash(const char* key)
{
	int i = 0;
	int sum = 0;
	while(key[i] != '\0')
	{
		sum += key[i];
		i++;
	}
	return sum % num_lists;
}

void MultiList_insert(SubList_t *lists, SortedListElement_t *element)
{
	if (element->key == NULL)
    {
        fprintf(stderr, "Error: element key is NULL\n");
        exit(2);
    }
	int lid = hash(element->key);
	SortedList_insert(lists[lid].list, element);
}

int MultiList_delete(SortedListElement_t *element)
{
	int rc = SortedList_delete(element);
	return rc;
}

SortedListElement_t *MultiList_lookup(SubList_t *lists, const char *key)
{
	int lid = hash(key);
	SortedListElement_t *ptr;
	ptr = SortedList_lookup(lists[lid].list, key);
	return ptr;
}

int MultiList_length(SubList_t *lists)
{
	int sum = 0;
	for (int i = 0; i < num_lists; i++)
	{
		int length = SortedList_length(lists[i].list);
		if (length == -1)
    	{
        	fprintf(stderr, "Error: Corrupted list\n");
        	exit(2);
    	}
		sum += length;
	}
	return sum;
}

void *list_none(void *args)
{
	int rc;
	long long * time_lock_total = malloc(sizeof(long long));
    if (time_lock_total == NULL)
    {
        fprintf(stderr, "Error: malloc failed\n");
        exit(2);
    }

	int id = *(int*)args;
	int start = id*num_iterats;
	int end = (id+1) * num_iterats;
	for (int i = start; i < end; i++)
	{
		MultiList_insert(lists, &element_array[i]);
	}
	int length = MultiList_length(lists);
	if (length == -1)
	{
		fprintf(stderr, "Error: Corrupted list\n");
		exit(2);
	}
	for (int i = start; i < end; i++)
	{
		SortedListElement_t *ptr;
		ptr = MultiList_lookup(lists, element_array[i].key);
		rc = MultiList_delete(ptr);
		if (rc)
		{
			fprintf(stderr, "Error: Corrupted list\n");
			exit(2);
		}
	}
	*time_lock_total = 0;
	pthread_exit((void*)time_lock_total);
	return NULL;
}

void *list_m(void *args)
{
	int rc;
	int lid;
	long long * time_lock_total = malloc(sizeof(long long));
    if (time_lock_total == NULL)
    {
        fprintf(stderr, "Error: malloc failed\n");
        exit(2);
    }

	int id = *(int*)args;
	int start = id*num_iterats;
	int end = (id+1) * num_iterats;
	for (int i = start; i < end; i++)
	{
		lid = hash(element_array[i].key);	
		rc = Pthread_mutex_lock(&lists[lid].lock_mutex, time_lock_total);
		if (rc) {exit(1);}
		MultiList_insert(lists, &element_array[i]);
		rc = pthread_mutex_unlock(&lists[lid].lock_mutex);
		if (rc) {exit(1);}
	}
	for (int i = 0; i < num_lists; i++)
	{
		rc = Pthread_mutex_lock(&lists[i].lock_mutex, time_lock_total);
    	if (rc) {exit(1);}
	}
	int length = MultiList_length(lists);
	for (int i = 0; i < num_lists; i++)
	{
		rc = pthread_mutex_unlock(&lists[i].lock_mutex);
    	if (rc) {exit(1);}
	}
	if (length == -1)
	{
		fprintf(stderr, "Error: Corrupted list\n");
		exit(2);
	}
	for (int i = start; i < end; i++)
	{
		lid = hash(element_array[i].key);
		SortedListElement_t *ptr;
		rc = Pthread_mutex_lock(&lists[lid].lock_mutex, time_lock_total);
        if (rc) {exit(1);}
		ptr = MultiList_lookup(lists, element_array[i].key);
		rc = MultiList_delete(ptr);
		if (rc)
		{
			fprintf(stderr, "Error: Corrupted list\n");
			exit(2);
		}
		rc = pthread_mutex_unlock(&lists[lid].lock_mutex);
        if (rc) {exit(1);}
	}
    pthread_exit((void*)time_lock_total);
	return NULL;
}

void *list_s(void *args)
{
    int rc;
	int lid;
	long long * time_lock_total = malloc(sizeof(long long));
	if (time_lock_total == NULL)
	{
		fprintf(stderr, "Error: malloc failed\n");
		exit(2);
	}
	
    int id = *(int*)args;
    int start = id*num_iterats;
    int end = (id+1) * num_iterats;
    for (int i = start; i < end; i++)
    {
		lid = hash(element_array[i].key);
        s_lock(&lists[lid].lock_spin, time_lock_total);
        MultiList_insert(lists, &element_array[i]);
        s_unlock(&lists[lid].lock_spin);
    }
	for (int i = 0; i < num_lists; i++)
	{
    	s_lock(&lists[i].lock_spin, time_lock_total);
	}
    int length = MultiList_length(lists);
	for (int i = 0; i < num_lists; i++)
	{
    	s_unlock(&lists[i].lock_spin);
	}
    if (length == -1)
    {
        fprintf(stderr, "Error: Corrupted list\n");
        exit(2);
    }
    for (int i = start; i < end; i++)
    {
		lid = hash(element_array[i].key);
        SortedListElement_t *ptr;
        s_lock(&lists[lid].lock_spin, time_lock_total);
        ptr = MultiList_lookup(lists, element_array[i].key);
        rc = MultiList_delete(ptr);
        if (rc)
        {
            fprintf(stderr, "Error: Corrupted list\n");
            exit(2);
        }
        s_unlock(&lists[lid].lock_spin);
    }
	/* printf("%lld\n", *time_lock_total); */
	pthread_exit((void*)time_lock_total);
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
		{"lists",		required_argument,	0,	4},
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
                }
                else
                {
                    fprintf(stderr, "Error: unrecognized option\n");
                    exit(1);
                }
                break;
			case 4:
				num_lists = strtol(optarg, NULL, 10);
				break;
			default:
				fprintf(stderr, "Error: Unrecognized option\n");	
				exit(1);
		}
	}

	/* initialize list */
	lists = (SubList_t*) malloc(sizeof(SubList_t)*num_lists);
	if (lists == NULL)
    {
        fprintf(stderr, "Error: malloc for lists failed\n");
        exit(1);
    }
	for (int i = 0; i < num_lists; i++)
	{
		SortedListElement_t *head;
		head = (SortedListElement_t*) malloc(sizeof(SortedListElement_t));
		if (head == NULL)
    	{
        	fprintf(stderr, "Error: malloc for head failed\n");
        	free(lists);
        	exit(1);
   		}
		head->key = NULL;
		head->next = head;
		head->prev = head;
		lists[i].list = head;
		pthread_mutex_init(&(lists[i].lock_mutex), NULL);
		s_init(&(lists[i].lock_spin));
	}

	/* create elements */
	int num_elements = num_threads * num_iterats;
	/* malloc elements */
	element_array = (SortedListElement_t*)
				malloc(sizeof(SortedListElement_t)*num_elements);
	if (element_array == NULL)
	{
		fprintf(stderr, "Error: malloc for element_array failed\n");
		free(lists);
		exit(1);
	}
	/* malloc keys */
	key_array = (char**) malloc(sizeof(char*)*num_elements);
	if (key_array == NULL)
	{
		fprintf(stderr, "Error: malloc for key_array failed\n");
		free(element_array);
		free(lists);
		exit(1);
	}
	/* initialize elements */
	for (int i = 0; i < num_elements; i++)
	{
		key_array[i] = (char*) malloc(sizeof(char)*256);
		if (key_array[i] == NULL)
		{
			fprintf(stderr, "Error: malloc for key failed\n");
			free(element_array);
			free(key_array);
			free(lists);
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
    if (thread_array == NULL)
    {
        fprintf(stderr, "Error: Malloc for thread_array failed\n");
		free(element_array);
        free(key_array);
        free(lists);
        exit(1);
    }

	/* create thread argument */
	int *ids = (int*) malloc(sizeof(int)*num_threads);
	if (ids == NULL)
	{
		fprintf(stderr, "Error: Malloc for ids failed\n");
		free(element_array);
        free(key_array);
        free(lists);
		free(thread_array);
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
			free(lists);
            exit(1);
        }
    }

	long long time_wait_total = 0;
	/* wait threads */
    for (int i = 0; i < num_threads; i++)
    {
		long long *time_wait_single;
        rc = pthread_join(thread_array[i], (void**)&time_wait_single);
        if (rc != 0 || time_wait_single == NULL)
        {
            fprintf(stderr, "pthread_join error!\n");
            free(thread_array);
			free(element_array);
        	free(key_array);
        	free(ids);
        	free(lists);
            exit(1);
        }
		/* printf("%lld\n", *time_wait_single); */
		time_wait_total += *time_wait_single;
		free(time_wait_single);
    }
	
	/* get end time */
    clock_gettime(CLOCK_MONOTONIC, &time_end);
    long long ssec = (long long) time_start.tv_sec;
    long long snsec = (long long) time_start.tv_nsec;
    long long esec = (long long) time_end.tv_sec;
    long long ensec = (long long) time_end.tv_nsec;
    time_total = (esec - ssec)*1e9 + (ensec - snsec);
	
	/* check length */
	int length = MultiList_length(lists);
	if (length != 0)
	{
		fprintf(stderr, "Error: non-zero failure\n");
		free(thread_array);
		free(element_array);
		free(key_array);
		free(ids);
		free(lists);
		exit(2);
	}	

	/* output */
    int num_ops = num_threads * num_iterats * 3;
    time_avg = time_total/num_ops;
	long long time_wait_per_lock = 
							time_wait_total/(num_elements * 2 + num_threads);


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
	
    fprintf(stdout, "%s,%d,%d,%d,%d,%lld,%lld,%lld\n", tag, num_threads,
                    num_iterats, num_lists, num_ops, time_total, time_avg, 
					time_wait_per_lock);

	for (int i = 0; i < num_lists; i++)
		pthread_mutex_destroy(&lists[i].lock_mutex);
	free(thread_array);
	free(element_array);
	free(key_array);
	free(ids);
	free(lists);
	return 0;
}
