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
#include<sched.h>

void SortedList_insert(SortedList_t *list, SortedListElement_t *element)
{
	if (element->key == NULL)
	{
		fprintf(stderr, "Error: element key is NULL\n");
		exit(2);
	}
	SortedListElement_t *curr;
	curr = list->next;
	
	while (curr->key != NULL)
	{
		if (strcmp(element->key, curr->key) > 0)
			curr = curr->next;
		else
			break;
	}

	if (opt_yield & INSERT_YIELD)
		sched_yield();

	element->prev = curr->prev;
	element->next = curr;
	element->prev->next = element;
	element->next->prev = element;
}

int SortedList_delete(SortedListElement_t *element)
{
	if (element->next->prev == element && element->prev->next == element)
	{
		if (opt_yield & DELETE_YIELD)
			sched_yield();
		element->prev->next = element->next;
		element->next->prev = element->prev;
		return 0;
	}
	else
	{
		return 1;
	}
}

SortedListElement_t *SortedList_lookup(SortedList_t *list, const char *key)
{
	SortedListElement_t *curr;
	curr = list->next;
	
	if (opt_yield & LOOKUP_YIELD)
		sched_yield();
	while(curr->key != NULL)
	{
		if (strcmp(key, curr->key) == 0)
			return curr;
		else if (strcmp(key, curr->key) < 0)
			return NULL;
		else
		{
			curr = curr->next;
		}
	}
	return NULL;
}

int SortedList_length(SortedList_t *list)
{
	int counter = 0;
	SortedListElement_t *curr;
	curr = list->next;
	if (opt_yield & LOOKUP_YIELD)
		sched_yield();
	while (curr->key != NULL)
	{
		if (curr->next->prev == curr && curr->prev->next == curr)
		{
			curr = curr->next;
			counter++;
		}
		else
			return -1;
	}
	return counter;
}
