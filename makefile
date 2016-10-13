# Makefile
# Lorenz Gerber
# Assignment 3 5DV088 HT16
# Makefile for mish - minimal shell
CC=gcc
CFLAGS= -Wall -pedantic -std=c11 -pthread

all: mfind

mfind: mfind.o list.o 
	$(CC) $(CFLAGS) -o mfind list.o mfind.o

mfind.o list.o: mfind.c mfind.h list.c list.h
	$(CC) $(CFLAGS) -c mfind.c list.c

clean:
	 rm -f rm *.o
