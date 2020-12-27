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

/**
 * SortedList_insert ... insert an element into a sorted list
 *
 *    The specified element will be inserted in to
 *    the specified list, which will be kept sorted
 *    in ascending order based on associated keys
 *
 * @param SortedList_t *list ... header for the list
 * @param SortedListElement_t *element ... element to be added to the list
 */
void SortedList_insert(SortedList_t *list, SortedListElement_t *element)
{
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
        pthread_yield();
    }
    element->next = curr;
    element->prev = curr->prev;
    curr->prev->next = element;
    curr->prev = element;
}

/**
 * SortedList_delete ... remove an element from a sorted list
 *
 *    The specified element will be removed from whatever
 *    list it is currently in.
 *
 *    Before doing the deletion, we check to make sure that
 *    next->prev and prev->next both point to this node
 *
 * @param SortedListElement_t *element ... element to be removed
 *
 * @return 0: element deleted successfully, 1: corrtuped prev/next pointers
 *
 */
int SortedList_delete(SortedListElement_t *element)
{
    if(element->next->prev == element->prev->next)
    {
        if(opt_yield & DELETE_YIELD)
        {
            pthread_yield();
        }
        element->prev->next = element->next;
        element->next->prev = element->prev;
        return 0;
    }
    return 1;
}

/**
 * SortedList_lookup ... search sorted list for a key
 *
 *    The specified list will be searched for an
 *    element with the specified key.
 *
 * @param SortedList_t *list ... header for the list
 * @param const char * key ... the desired key
 *
 * @return pointer to matching element, or NULL if none is found
 */
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
            pthread_yield();
        }
        curr = curr->next;
    }
    return NULL;
}

/**
 * SortedList_length ... count elements in a sorted list
 *    While enumerating list, it checks all prev/next pointers
 *
 * @param SortedList_t *list ... header for the list
 *
 * @return int number of elements in list (excluding head)
 *       -1 if the list is corrupted
 */
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
            pthread_yield();
        }
        curr = curr->next;
    }
    return count;
}

