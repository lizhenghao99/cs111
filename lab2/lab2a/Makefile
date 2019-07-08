# NAME:		Zhenghao Li
# EMAIL:	lizhenghao99@g.ucla.edu
# ID:		704971934

CC=gcc
CFLAGS=-g -O0 -Wall -Wextra -pthread

default: build

build: lab2_add lab2_list

lab2_add: lab2_add.o
	$(CC) $(CFLAGS) lab2_add.o -o $@

lab2_add.o: lab2_add.c
	$(CC) $(CFLAGS) -c lab2_add.c -o $@

lab2_list: lab2_list.o SortedList.o
	$(CC) $(CFLAGS) lab2_list.o SortedList.o -o $@

lab2_list.o: lab2_list.c SortedList.h
	$(CC) $(CFLAGS) -c lab2_list.c -I. -o $@

SortedList.o: SortedList.c SortedList.h
	$(CC) $(CFLAGS) -c SortedList.c -I. -o $@

tests: test.sh
	rm -f lab2_add.csv
	rm -f lab2_list.csv
	chmod +x test.sh
	-./test.sh 2> /dev/null	

graphs: lab2_add.gp lab2_list.gp
	gnuplot lab2_add.gp	
	gnuplot lab2_list.gp

dist: test.sh lab2_add.gp lab2_list.gp build tests graphs
	tar -czf lab2a-704971934.tar.gz lab2_add.c SortedList.h SortedList.c \
		lab2_list.c Makefile lab2_add.csv lab2_list.csv lab2_add-1.png \
		lab2_add-2.png lab2_add-3.png lab2_add-4.png lab2_add-5.png \
		lab2_list-1.png lab2_list-2.png lab2_list-3.png lab2_list-4.png \
		README test.sh lab2_add.gp lab2_list.gp
	

clean: 
	rm -f lab2_add
	rm -f lab2_add.o
	rm -f lab2_list
	rm -f lab2_list.o
	rm -f SortedList.o
	rm -f lab2a-704971934.tar.gz
