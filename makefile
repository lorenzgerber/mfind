# Makefile
# Lorenz Gerber
# Assignment 3 5DV088 HT16
# Makefile for mish - minimal shell
CC=gcc
CFLAGS= -Wall -pedantic -std=c11 -Wextra -Werror -Wmissing-declarations -Wmissing-prototypes -Werror-implicit-function-declaration -Wreturn-type -Wparentheses -Wunused -Wold-style-definition -Wundef -Wshadow -Wstrict-prototypes -Wswitch-default -Wstrict-prototypes -Wunreachable-code

all: mfind

mish: mfind.o list.o 
	$(CC) $(CFLAGS) -o mfind list.o mfind.o

mfind.o list.o: mfind.c list.c list.h
	$(CC) $(CFLAGS) -c mfind.c list.c

clean:
	 rm -f rm *.o
