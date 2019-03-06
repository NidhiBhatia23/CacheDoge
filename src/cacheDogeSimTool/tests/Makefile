#
# Created on Mon Nov 19 2018
# Author: Artur Balanuta & Nidhi Bhatia
# Copyright (c) 2018 Carnegie Mellon University
#

CC=gcc
CFLAGS += -pthread -g -O3
LDFLAGS= -pthread

all: simple simple_1th smashCache goodCachees

simple:
	$(CC) -c $(CFLAGS) -o simple.o simple.c
	$(CC) $(LDFLAGS) -o simple.x ./simple.o

simple_1th:
	$(CC) -c -O0 -o simple_1th.o simple_1th.c
	$(CC) -o simple_1th.x ./simple_1th.o

smashCache:
	 $(CC) -c $(CFLAGS) -O0 -o smashCache.o smashCache.c
	 $(CC) $(LDFLAGS) -o smashCache.x ./smashCache.o

goodCache:
	 $(CC) -c $(CFLAGS) -O0 -o goodCache.o goodCache.c
	 $(CC) $(LDFLAGS) -o goodCache.x ./goodCache.o

goodCachees:
	 $(CC) -c $(CFLAGS) -O0 -o goodCachees.o goodCachees.c
	 $(CC) $(LDFLAGS) -o goodCachees.x ./goodCachees.o

clean:
	-rm *.o
	-rm simple.x
	-rm simple_1th.x
	-rm smashCache.x
	-rm goodCache.x
	-rm goodCachees.x