//
//  lab2_add.c
//  
//
//  Created by Jeannie Chiem on 11/1/17.
//

#define _GNU_SOURCE

#include <getopt.h>
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <string.h>
#include <time.h>

int iters = 1;
int opt_yield = 0;
int spin_flag = 0;
pthread_mutex_t mutex;
char syncopt;

void add(long long *pointer, long long value)
{
    long long sum = *pointer + value;
    if (opt_yield)
        sched_yield();
    *pointer = sum;
}

void add_compare(long long *pointer, long long value)
{
    long long old = *pointer;
    long long sum = old + value;
    if(opt_yield)
    {
        sched_yield();
    }
    while(__sync_val_compare_and_swap(pointer, old, sum) != old)
    {
        old = *pointer;
        sum = old + value;
        if(opt_yield)
        {
            sched_yield();
        }
    }
}

void* t_add(void* value)
{
    for (int i = 0; i < iters; i ++)
    {
        if(syncopt == 'm')
        {
            pthread_mutex_lock(&mutex);
            add((long long*)value, 1);
            pthread_mutex_unlock(&mutex);
        }
        else if (syncopt == 's')
        {
            while(__sync_lock_test_and_set(&spin_flag, 1));
            add((long long *)value, 1);
            __sync_lock_release(&spin_flag);
        }
        else if (syncopt == 'c')
        {
            add_compare((long long*)value, 1);
        }
        else
        {
            add((long long*)value, 1);
        }
    }
    
    for (int i = 0; i < iters; i ++)
    {
        if(syncopt == 'm')
        {
            pthread_mutex_lock(&mutex);
            add((long long*)value, -1);
            pthread_mutex_unlock(&mutex);
        }
        else if (syncopt == 's')
        {
            while(__sync_lock_test_and_set(&spin_flag, 1));
            add((long long *)value, -1);
            __sync_lock_release(&spin_flag);
        }
        else if (syncopt == 'c')
        {
            add_compare((long long*)value, -1);
        }
        else
        {
            add((long long*)value, -1);
        }
    }
    
    return NULL;
}

int main(int argc, char* argv[])
{
    int opt = 0, threadnum = 1;
    int s_flag = 0;
    char testname[20];
    
    static struct option long_options[] =
    {
        {"threads", required_argument, 0, 't'},
        {"iterations", required_argument, 0, 'i'},
        {"yield", no_argument, 0, 'y'},
        {"sync", required_argument, 0, 's'},
        {0,0,0,0}
    };
    
    while((opt = getopt_long(argc, argv, "t:i:ys:", long_options, NULL)) != -1)
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
                opt_yield = 1;
                strcpy(testname, "-yield");
                break;
            case 's':
                s_flag = 1;
                if(strlen(optarg) == 1)
                {
                    if(optarg[0] == 'm' || optarg[0] == 's' || optarg[0] == 'c')
                        syncopt = optarg[0];
                    else
                    {
                        fprintf(stderr, "ERROR: invalid argument for sync\n options:\n\tm = mutex\n\ts = spin-lock\n\tc = compare and swap\n");
                        exit(1);
                    }
                }
                else
                {
                    fprintf(stderr, "ERROR: invalid argument for sync\n options:\n\tm = mutex\n\ts = spin-lock\n\tc = compare and swap\n");
                    exit(1);
                }
                break;
            default:
                fprintf(stderr, "ERROR: invalid argument\n Usage: lab2_add --threads=# --iterations=# --yield --sync=[msc]\n");
                exit(1);
        }
    }
    
    if(opt_yield == 0)
    {
        if(s_flag == 0)
            strcpy(testname, "-none");
        else
            sprintf(testname, "-%c", syncopt);
    }
    else if(opt_yield == 1)
    {
        if(s_flag == 0)
            strcat(testname, "-none");
        else
            sprintf(testname, "%s-%c", testname, syncopt);
    }
    
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
        if(pthread_create(threads + i, NULL, t_add, &counter))
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
   
    long run_time = (1000000000 * (t2.tv_sec - t1.tv_sec)) + (t2.tv_nsec - t1.tv_nsec);
    int operations = (threadnum * iters * 2);
    long avg_time = (run_time/operations);
    fprintf(stdout, "add%s,%d,%d,%d,%ld,%ld,%lld\n", testname, threadnum, iters, operations, run_time, avg_time, counter);
    
    if(counter)
    {
        exit(1);
    }
    else
    {
        exit(0);
    }
    
}
