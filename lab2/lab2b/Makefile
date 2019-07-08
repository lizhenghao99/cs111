# NAME:		Zhenghao Li
# EMAIL:	lizhenghao99@g.ucla.edu
# ID:		704971934

CC=gcc
CFLAGS=-g -O0 -Wall -Wextra -pthread

default: build

build: lab2_list

lab2_list: lab2b_list.o SortedList.o
	$(CC) $(CFLAGS) -lprofiler lab2b_list.o SortedList.o -o $@

lab2b_list.o: lab2_list.c SortedList.h
	$(CC) $(CFLAGS) -c lab2_list.c -I. -o $@

SortedList.o: SortedList.c SortedList.h
	$(CC) $(CFLAGS) -c SortedList.c -I. -o $@

tests: test.sh
	rm -f lab2b_list.csv
	chmod +x test.sh
	-./test.sh 2> /dev/null	

profile:
	rm -f profile.out
	LD_PRELOAD=/usr/lib64/libprofiler.so CPUPROFILE=/tmp/lab2_list.gprof \
	./lab2_list --threads=12 --iterations=1000 --sync=s
	pprof ./lab2_list /tmp/lab2_list.gprof > profile.out
	pprof --list=s_lock ./lab2_list /tmp/lab2_list.gprof >> profile.out

graphs: lab2b_list.gp
	gnuplot lab2b_list.gp

dist: test.sh lab2b_list.gp build tests graphs profile
	tar -czf lab2b-704971934.tar.gz SortedList.h SortedList.c \
		lab2_list.c Makefile lab2b_list.csv \
		lab2b_1.png lab2b_2.png lab2b_3.png lab2b_4.png lab2b_5.png \
		README test.sh lab2b_list.gp profile.out
	

clean: 
	rm -f lab2_list
	rm -f lab2b_list.o
	rm -f SortedList.o
	rm -f lab2b-704971934.tar.gz
