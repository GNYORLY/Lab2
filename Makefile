#!/bin/bash
#Jeannie Chiem
#'gnyorly@gmail.com'
#ID:504666652

CC = gcc
CFLAGS = -Wall -Wextra
ARCHIVE = lab2a-504666652.tar.gz
sources = Makefile README lab2_add.c lab2_list.c SortedList.c SortedList.h test.sh *.png *.csv

all: build

build: lab2_add lab2_list

list_files = SortedList.o lab2_list.o
lab2_list: $(list_files)
	@$(CC) $(CFLAGS) -o $@ $(list_files)

lab2_list.o SortedList.o: SortedList.h

add_files = lab2_add.c
lab2_add: $(add_files)
	@$(CC) $(CFLAGS) -o $@ $(add_files)

tests: build
	@chmod +x test.sh
	@-./test.sh

graphs:
	@gnuplot lab2_add.gp
	@gnuplot lab2_list.gp

dist:
	@tar -czvf $(ARCHIVE) $(sources)

clean:
	@rm -f lab2_add lab2_list *.o *.png *.csv
