//
//  lab2_list.c
//  
//
//  Created by Jeannie Chiem on 11/1/17.
//

#define _POSIX_C_SOURCE 199309L

#include "SortedList.h"

#include <pthread.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <time.h>

typedef struct {
    SortedList_t head;
    pthread_mutex_t mutex;
    int spinlock;
} SubList;

static SubList *list;
static SortedListElement_t *element;

static int keysize = 5;
int opt_yield = 0;

long tmutex = 0;
int iters = 1;
int threadnum = 1;
int listnum = 1;
int iflag = 0;
int dflag = 0;
int lflag = 0;
char syncopt = '0';

void initLists()
{
    list = malloc(sizeof(SubList) * listnum);
    for(int i = 0; i < listnum; i++)
    {
        SubList *sublist = &list[i];
        SortedList_t *list = &sublist->head;
        list->key = NULL;
        list->next = list;
        list->prev = list;
        if (syncopt == 's')
        {
            sublist->spinlock = 0;
        }
        else if (syncopt == 'm')
        {
            pthread_mutex_init(&sublist->mutex, NULL);
        }
    }
}

void keygen(int size)
{
    for(int i = 0; i < size; i++)
    {
        char *key = malloc(sizeof(char) * (keysize+1));
        for (int j = 0; j < keysize; j++)
        {
            key[j] = (char)rand() % 26 + 'A';
        }
        key[keysize] = '\0';
        element[i].key = key;
    }
}

//multiplicative hash function
unsigned keyHash(const char *key)
{
    unsigned hash = 5381;
    for(unsigned i = 0; i < (unsigned)keysize; ++i)
        hash = 33 * hash + key[i];
    return hash % listnum;
}
               
void* threadjob(void* thread)
{
    unsigned start = *((int *)thread);
    unsigned end = start + iters;
    SortedListElement_t *tele;
    SubList *sublist;
    pthread_mutex_t *mutex;
    int *spin;
    struct timespec t1, t2;
    
    for(unsigned i = start; i < end; i++)
    {
        tele = &element[i];
        const char *key = tele->key;
        sublist = &list[keyHash(key)];
        
        if(syncopt == 'm')
        {
            mutex = &sublist->mutex;
            clock_gettime(CLOCK_MONOTONIC, &t1);
            pthread_mutex_lock(mutex);
            clock_gettime(CLOCK_MONOTONIC, &t2);
            tmutex += (1000000000 * (t2.tv_sec - t1.tv_sec)) + (t2.tv_nsec - t1.tv_nsec);
            
            SortedList_insert(&sublist->head, tele);
            pthread_mutex_unlock(mutex);
        }
        else if (syncopt == 's')
        {
            spin = &sublist->spinlock;
            while(__sync_lock_test_and_set(spin, 1))
            {
                continue;
            }
            SortedList_insert(&sublist->head, tele);
            __sync_lock_release(spin);
        }
        else
        {
            SortedList_insert(&sublist->head, tele);
        }
    }
    
    for(int i = 0; i < listnum; i++)
    {
        if(syncopt == 'm')
        {
            clock_gettime(CLOCK_MONOTONIC, &t1);
            pthread_mutex_lock(&list[i].mutex);
            clock_gettime(CLOCK_MONOTONIC, &t2);
            tmutex += (1000000000 * (t2.tv_sec - t1.tv_sec)) + (t2.tv_nsec - t1.tv_nsec);
            
            SortedList_length(&list[i].head);
            pthread_mutex_unlock(&list[i].mutex);
        }
        else if (syncopt == 's')
        {
            while(__sync_lock_test_and_set(&list[i].spinlock, 1))
            {
                continue;
            }
            SortedList_length(&list[i].head);
            __sync_lock_release(&list[i].spinlock);
        }
        else
        {
            SortedList_length(&list[i].head);
        }
    }
    
    SortedListElement_t *temp;
    for(unsigned i = start; i < end; i++)
    {
        tele = &element[i];
        const char *key = tele->key;
        sublist = &list[keyHash(key)];
        
        if(syncopt == 'm')
        {
            mutex = &sublist->mutex;
            
            clock_gettime(CLOCK_MONOTONIC, &t1);
            pthread_mutex_lock(mutex);
            clock_gettime(CLOCK_MONOTONIC, &t2);
            tmutex += (1000000000 * (t2.tv_sec - t1.tv_sec)) + (t2.tv_nsec - t1.tv_nsec);
            
            temp = SortedList_lookup(&sublist->head, key);
            SortedList_delete(temp);
            pthread_mutex_unlock(mutex);
        }
        else if (syncopt == 's')
        {
            spin = &sublist->spinlock;
            while(__sync_lock_test_and_set(spin, 1))
            {
                continue;
            }
            temp = SortedList_lookup(&sublist->head, key);
            SortedList_delete(temp);
            __sync_lock_release(spin);
        }
        else
        {
            temp = SortedList_lookup(&sublist->head, key);
            SortedList_delete(temp);
        }
    }
    
    return NULL;
}

int main(int argc, char* argv[])
{
    int opt = 0, s_flag = 0;
    int listlen = 0;
    char yields[10];
    char yieldopts[10] = "-";
    char syncopts[10] = "-";
    
    static struct option long_options[] =
    {
        {"threads", required_argument, 0, 't'},
        {"iterations", required_argument, 0, 'i'},
        {"yield", required_argument, 0, 'y'},
        {"sync", required_argument, 0, 's'},
        {"lists", required_argument, 0, 'l'},
        {0,0,0,0}
    };
    
    int ycount = 0;
    while((opt = getopt_long(argc, argv, "t:i:y:s:l:", long_options, NULL)) != -1)
    {
        switch(opt)
        {
            case 't':
                threadnum = atoi(optarg);
                break;
            case 'i':
                iters = atoi(optarg);
                break;
            case 'l':
                listnum = atoi(optarg);
                break;
            case 'y':
                ycount = strlen(optarg);
                if(ycount <= 3)
                {
                    strcpy(yields, optarg);
                    for(int i = 0; i < ycount; i++)
                    {
                        if(yields[i] == 'i' && iflag == 0)
                        {
                            iflag = 1;
                            opt_yield |= INSERT_YIELD;
                        }
                        else if(yields[i] == 'd' && dflag == 0)
                        {
                            dflag = 1;
                            opt_yield |= DELETE_YIELD;
                        }
                        else if(yields[i] == 'l' && lflag == 0)
                        {
                            lflag = 1;
                            opt_yield |= LOOKUP_YIELD;
                        }
                        else
                        {
                            fprintf(stderr, "ERROR: invalid argument for yield\n options:\n\ti = insert\n\td = delete\n\tl = lookups\n");
                        }
                    }
                }
                else
                {
                    fprintf(stderr, "ERROR: invalid argument for yield\n options:\n\ti = insert\n\td = delete\n\tl = lookups\n");
                }
                break;
            case 's':
                if(strlen(optarg) == 1)
                {
                    if(optarg[0] == 'm' || optarg[0] == 's')
                        syncopt = optarg[0];
                    else
                    {
                        fprintf(stderr, "ERROR: invalid argument for sync\n options:\n\tm = mutex\n\ts = spin-lock\n");
                        exit(1);
                    }
                    s_flag = 1;
                }
                else
                {
                    fprintf(stderr, "ERROR: invalid argument for sync\n options:\n\tm = mutex\n\ts = spin-lock\n");
                    exit(1);
                }
                break;
            default:
                fprintf(stderr, "ERROR: invalid argument\n usage: lab2_list --threads=# --iterations=# --yield=[idl] --sync=[ms] --lists=#\n");
                exit(1);
        }
    }
    
    if(iflag == 0 && dflag == 0 && lflag == 0)
    {
        strcat(yieldopts, "none");
    }
    else
    {
        if(iflag == 1)
        {
            strcat(yieldopts, "i");
        }
        if(dflag == 1)
        {
            strcat(yieldopts, "d");
        }
        if(lflag == 1)
        {
            strcat(yieldopts, "l");
        }
    }
    
    if(s_flag == 1)
    {
        sprintf(syncopts, "%s%c", syncopts, syncopt);
    }
    else
    {
        strcat(syncopts, "none");
    }
    
    int listsize = iters * threadnum;
    initLists();
    element = malloc(sizeof(SortedListElement_t) * listsize);
    keygen(listsize);
    
    pthread_t *threads = malloc(sizeof(pthread_t) * threadnum);
    int *tid = malloc(sizeof(int) * threadnum);
    int t = 0;
    for (int i = 0; i < threadnum; i++)
    {
        tid[i] = t;
        t += iters;
    }
    
    struct timespec first, last;
    
    if(clock_gettime(CLOCK_MONOTONIC, &first) == -1)
    {
        perror("ERROR: failed clock_gettime 1\n");
        exit(1);
    }
    
    int k = 0;
    for(int i = 0; i < threadnum; i++)
    {
        tid[i] = k;
        int check = pthread_create(&threads[i], NULL, threadjob, (void*)&tid[i]);
        if(check)
        {
            perror("ERROR: failed pthread_create\n");
            exit(1);
        }
        k += iters;
    }
    
    for(int i = 0; i < threadnum; i++)
    {
        fprintf(stderr, "watabout love\n");
        int check = pthread_join(threads[i], NULL);
        fprintf(stderr, "watabout us\n");
        if(check)
        {
            perror("ERROR: failed pthread_join\n");
            exit(1);
        }
    }
    
    if(clock_gettime(CLOCK_MONOTONIC, &last) == -1)
    {
        perror("ERROR: failed clock_gettime 2\n");
        exit(1);
    }
    
    for(int i = 0; i < listnum; i++)
    {
        listlen += SortedList_length(&list[i].head);
    }
    
    long run_time = (1000000000 * (last.tv_sec - first.tv_sec)) + (last.tv_nsec - first.tv_nsec);
    int operations = (threadnum * iters * 3);
    long avg_time = (run_time/operations);
    long avgmutex = (tmutex/operations);
    fprintf(stdout, "list%s%s,%d,%d,%d,%d,%ld,%ld,%ld\n", yieldopts, syncopts, threadnum, iters, listnum, operations, run_time, avg_time, avgmutex);
    
    free(threads);
    free(element);
    exit(0);
}
