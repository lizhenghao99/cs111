# NAME: 	Zhenghao Li
# EMAIL: 	lizhenghao99@g.ucla.edu
# ID:		704971934

CC=gcc
CFLAGS=-g -Wall -Wextra

default: lab0

lab0.o: lab0.c
	$(CC) $(CFLAGS) -c lab0.c -o $@

lab0: lab0.o
	$(CC) $(CFLAGS) lab0.o -o $@

check: check.sh
	./check.sh
dist: lab0.c Makefile backtrace.png breakpoint.png README check.sh
	tar -czf ./lab0-704971934.tar.gz lab0.c Makefile backtrace.png breakpoint.png README check.sh

clean: 
	rm -f lab0.o
	rm -f lab0
	rm -f lab0-704971934.tar.gz
 
