# NAME:		Zhenghao Li
# EMAIL:	lizhenghao99@g.ucla.edu
# ID:		704971934

CC=gcc
CFLAGS=-Wall -Wextra

default: lab3a

lab3a: lab3a.o
	$(CC) $(CFLAGS) lab3a.o -o $@

lab3a.o: lab3a.c
	$(CC) $(CFLAGS) -c lab3a.c -o $@

dist: lab3a.c ext2_fs.h Makefile README
	tar -czf lab3a-704971934.tar.gz lab3a.c ext2_fs.h Makefile README

clean:
	rm -f lab3a
	rm -f lab3a.o
	rm -f lab3a-704971934.tar.gz
