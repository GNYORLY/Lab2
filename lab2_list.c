//
//  lab2_list.c
//  
//
//  Created by Jeannie Chiem on 11/1/17.
//


#include "SortedList.h"

#include <pthread.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <time.h>
#include <sched.h>

static SortedList_t list;
static SortedListElement_t *element;
pthread_mutex_t mutex;

static int keysize = 5;
int opt_yield = 0;

int spin_flag = 0;
int iters = 1;
int threadnum = 1;
static int lists = 1;
int iflag = 0;
int dflag = 0;
int lflag = 0;
char syncopt = '0';

void initList(int size)
{
    if(size < 1)
        return;
    for(int i = 0; i < size; i++)
    {
        char *key = malloc(sizeof(char) * (keysize+1));
        for (int j = 0; j < keysize; j++)
        {
            key[j] = rand();
        }
        key[keysize] = '\0';
        element[i].key = key;
    }
}

void* threadjob(void* count)
{
    
    for(int i = 0; i < iters; i++)
    {
        if(syncopt == 'm')
        {
            pthread_mutex_lock(&mutex);
            SortedList_insert(&list, &element[i]);
            pthread_mutex_unlock(&mutex);
        }
        else if (syncopt == 's')
        {
            while(__sync_lock_test_and_set(&spin_flag, 1));
            SortedList_insert(&list, &element[i]);
            __sync_lock_release(&spin_flag);
        }
        else
        {
            SortedList_insert(&list, &element[i]);
        }
    }
    
    if(syncopt == 'm')
    {
        pthread_mutex_lock(&mutex);
        SortedList_length(&list);
        pthread_mutex_unlock(&mutex);
    }
    else if (syncopt == 's')
    {
        while(__sync_lock_test_and_set(&spin_flag, 1));
        SortedList_length(&list);
        __sync_lock_release(&spin_flag);
    }
    else
    {
        SortedList_length(&list);
    }
    
    SortedListElement_t *temp;
    for(int i = 0; i < iters; i++)
    {
        if(syncopt == 'm')
        {
            pthread_mutex_lock(&mutex);
            temp = SortedList_lookup(&list, element[i].key);
            if(SortedList_delete(temp))
            {
                perror("ERROR: failed SortedList_delete\n");
                exit(1);
            }
            pthread_mutex_unlock(&mutex);
        }
        else if (syncopt == 's')
        {
            while(__sync_lock_test_and_set(&spin_flag, 1));
            temp = SortedList_lookup(&list, element[i].key);
            if(SortedList_delete(temp))
            {
                perror("ERROR: failed SortedList_delete\n");
                exit(1);
            }
            __sync_lock_release(&spin_flag);
        }
        else
        {
            temp = SortedList_lookup(&list, element[i].key);
            if(SortedList_delete(temp))
            {
                perror("ERROR: failed SortedList_delete\n");
                exit(1);
            }
        }
    }
    
    return NULL;
}

int main(int argc, char* argv[])
{
    int opt = 0;
    int s_flag = 0;
    char yields[10];
    char yieldopts[10] = "-";
    char syncopts[10] = "-";
    int ycount;
    
    static struct option long_options[] =
    {
        {"threads", required_argument, 0, 't'},
        {"iterations", required_argument, 0, 'i'},
        {"yield", required_argument, 0, 'y'},
        {"sync", required_argument, 0, 's'},
        {0,0,0,0}
    };
    
    while((opt = getopt_long(argc, argv, "t:i:y:s:", long_options, NULL)) != -1)
    {
        switch(opt)
        {
            case 't':
                threadnum = atoi(optarg);
                break;
            case 'i':
                iters = atoi(optarg);
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
                    if(optarg[0] == 'm' || optarg[0] == 's' || optarg[0] == 'c')
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
                fprintf(stderr, "ERROR: invalid argument\n Usage: lab2_list --threads=# --iterations=# --yield=[idl] --sync=[ms]\n");
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
    SortedList_t* head = &list;
    list.key = NULL;
    list.next = head;
    list.prev = head;
    
    element = malloc(sizeof(SortedListElement_t) * listsize);
    initList(listsize);
    
    pthread_t *threads = malloc(threadnum * sizeof(pthread_t));
    long long counter = 0;
    struct timespec t1, t2;
    
    if(clock_gettime(CLOCK_MONOTONIC, &t1) == -1)
    {
        perror("ERROR: failed clock_gettime 1\n");
        exit(1);
    }
    
    for(int i = 0; i < threadnum; i++)
    {
        if(pthread_create(threads + i, NULL, threadjob, &counter))
        {
            perror("ERROR: failed pthread_create\n");
            exit(1);
        }
    }
    
    for(int i = 0; i < threadnum; i++)
    {
        if(pthread_join(*(threads + i), NULL))
        {
            perror("ERROR: failed pthread_join\n");
            exit(1);
        }
    }
    
    if(clock_gettime(CLOCK_MONOTONIC, &t2) == -1)
    {
        perror("ERROR: failed clock_gettime 2\n");
        exit(1);
    }
    
    int listcheck = SortedList_length(&list);
    if(listcheck)
    {
        perror("ERROR: final list length is not 0\n");
        exit(1);
    }
    
    long run_time = (1000000000 * (t2.tv_sec - t1.tv_sec)) + (t2.tv_nsec - t1.tv_nsec);
    int operations = (threadnum * iters * 3);
    long avg_time = (run_time/operations);
    fprintf(stdout, "list%s%s,%d,%d,%d,%d,%ld,%ld\n", yieldopts, syncopts, threadnum, iters, lists, operations, run_time, avg_time);
    
}
