//
//  SortedList.c
//  
//
//  Created by Jeannie Chiem on 11/1/17.
//

#define _GNU_SOURCE
#include "SortedList.h"
#include <pthread.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sched.h>


void SortedList_insert(SortedList_t *list, SortedListElement_t *element)
{
    if(list == NULL || element == NULL)
    {
        return;
    }
    SortedListElement_t *curr = list->next;
    while(curr != list)
    {
        if(strcmp(element->key, curr->key) <= 0)
        {
            break;
        }
        curr  = curr->next;
    }
    
    if(opt_yield & INSERT_YIELD)
    {
        sched_yield();
    }
    element->next = curr;
    element->prev = curr->prev;
    curr->prev->next = element;
    curr->prev = element;
}


int SortedList_delete(SortedListElement_t *element)
{
    if(element == NULL)
    {
        return -1;
    }
    if(element->next->prev == element->prev->next)
    {
        if(opt_yield & DELETE_YIELD)
        {
            sched_yield();
        }
        element->prev->next = element->next;
        element->next->prev = element->prev;
        return 0;
    }
    return 1;
}


SortedListElement_t *SortedList_lookup(SortedList_t *list, const char *key)
{
    SortedListElement_t *curr = list->next;
    while(curr != list)
    {
        if(strcmp(curr->key, key) == 0)
        {
            return curr;
        }
        if(opt_yield & LOOKUP_YIELD)
        {
            sched_yield();
        }
        curr = curr->next;
    }
    return NULL;
}


int SortedList_length(SortedList_t *list)
{
    if(list == NULL)
    {
        return -1;
    }
    int count = 0;
    SortedListElement_t *curr = list->next;
    while(curr != list)
    {
        count++;
        if(opt_yield & LOOKUP_YIELD)
        {
            sched_yield();
        }
        curr = curr->next;
    }
    return count;
}

