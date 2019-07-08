# NAME:		Zhenghao Li
# EMAIL:	lizhenghao99@g.ucla.edu
# ID:		704971934

CC=gcc
CFLAGS=-std=c99 -Wall -Wextra -lmraa -lm -pthread

default: lab4c_tcp lab4c_tls

lab4c_tcp: lab4c_tcp.o
	$(CC) $(CFLAGS) lab4c_tcp.o -o $@

lab4c_tcp.o: lab4c_tcp.c
	$(CC) $(CFLAGS) -c lab4c_tcp.c -o $@

lab4c_tls: lab4c_tls.o
	$(CC) $(CFLAGS) -lssl -lcrypto lab4c_tls.o -o $@

lab4c_tls.o: lab4c_tls.c
	$(CC) $(CFLAGS) -lssl -lcrypto -c lab4c_tls.c -o $@


dist: lab4c_tcp.c lab4c_tls.c Makefile README
	tar -czf lab4c-704971934.tar.gz lab4c_tcp.c lab4c_tls.c Makefile README

clean:
	rm -f lab4c_tcp
	rm -f lab4c_tcp.o
	rm -f lab4c_tls
	rm -f lab4c_tls.o
	rm -f lab4c-704971934.tar.gz
