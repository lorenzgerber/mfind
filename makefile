# Makefile
# Lorenz Gerber
# Assignment 3 5DV088 HT16
# Makefile for mish - minimal shell
CC=gcc
CFLAGS= -Wall -pedantic -std=c11 -Wextra -Werror -Wmissing-declarations -Wmissing-prototypes -Werror-implicit-function-declaration -Wreturn-type -Wparentheses -Wunused -Wold-style-definition -Wundef -Wshadow -Wstrict-prototypes -Wswitch-default -Wstrict-prototypes -Wunreachable-code

all: mish

mish: mish.o execute.o parser.o sighant.o
	$(CC) $(CFLAGS) -o mish sighant.o execute.o parser.o mish.o

mish.o execute.o parser.o: sighant.c sighant.h parser.c parser.h execute.c execute.h mish.c mish.h
	$(CC) $(CFLAGS) -c -g sighant.c parser.c execute.c mish.c

clean:
	 rm -f rm mish *.o
