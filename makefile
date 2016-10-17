# Makefile
# Lorenz Gerber
# Assignment 3 5DV088 HT16
# Makefile for mish - minimal shell
CC=gcc
CFLAGS= -Wall -pedantic -std=c11 -pthread -D_SVID_SOURCE

all: mfind

mfind: mfind.o list.o semops.o 
	$(CC) $(CFLAGS) -o mfind list.o mfind.o semops.o

mfind.o: mfind.c mfind.h
	 $(CC) $(CFLAGS) -c mfind.c

list.o: list.c list.h
	$(CC) $(CFLAGS) -c list.c

semops.o: semops.c semops.h
	$(CC) $(CFLAGS) -c semops.c

clean:
	 rm -f rm *.o
