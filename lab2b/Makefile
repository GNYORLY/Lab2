#!/bin/bash
#Jeannie Chiem
#'gnyorly@gmail.com'
#ID:504666652

CC = gcc
CFLAGS = -Wall -Wextra -std=gnu11 -pthread -g -lprofiler
ARCHIVE = lab2b-504666652.tar.gz
sources = Makefile README lab2_list.c SortedList.c SortedList.h test.sh lab2_list.gp profile.out *.png *.csv *.gp

all: build

build: lab2_list

list_files = lab2_list.c SortedList.c
lab2_list: $(list_files)
	@$(CC) $(CFLAGS) -o $@ $(list_files)

tests: build
	@chmod +x test.sh
	@-./test.sh


profile: profile.out

thread_func = threadjob
profile.out: build
	@export CPUPROFILE=~/profile.out
	@$(CC) $(CFLAGS) -o lab2_list lab2_list.c SortedList.c
	./lab2_list --threads=12 --iterations=1000 --sync=s
	@pprof --list=$(thread_func) ~/lab2_list ~/profile.out > temp.out
	@cp temp.out profile.out
	@rm -f temp.out


graphs:
	@gnuplot lab2_list.gp

dist:
	@tar -czvf $(ARCHIVE) $(sources)

clean:
	@rm -f lab2_list *.png *.csv *.out

