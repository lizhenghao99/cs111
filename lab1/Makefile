# NAME:		Zhenghao Li
# EMAIL: 	lizhenghao99@g.ucla.edu
# ID:		704971934


CC=gcc
CFLAGS=-g -Wall -Wextra

default: simpsh

simpsh:	simpsh.o
	$(CC) $(CFLAGS) simpsh.o -o $@

simpsh.o: simpsh.c
	$(CC) $(CFLAGS) -c simpsh.c -o $@

check: test.sh
	./test.sh
dist: simpsh.c test.sh Makefile README
	tar -czf lab1-704971934.tar.gz simpsh.c test.sh Makefile README

clean:
	rm -f simpsh.o
	rm -f simpsh
	rm -f lab1-704971934.tar.gz
