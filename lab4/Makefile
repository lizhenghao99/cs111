# NAME:		Zhenghao Li
# EMAIL:	lizhenghao99@g.ucla.edu
# ID:		704971934

CC=gcc
CFLAGS=-std=c99 -Wall -Wextra -lmraa -lm -pthread

default: lab4b

lab4b: lab4b.o
	$(CC) $(CFLAGS) lab4b.o -o $@

lab4b.o: lab4b.c
	$(CC) $(CFLAGS) -c lab4b.c -o $@

check: smoke.sh
	chmod +x smoke.sh
	./smoke.sh

dist: lab4b.c smoke.sh Makefile README
	tar -czf lab4b-704971934.tar.gz lab4b.c smoke.sh Makefile README

clean:
	rm -f lab4b
	rm -f lab4b.o
	rm -f lab4b-704971934.tar.gz
