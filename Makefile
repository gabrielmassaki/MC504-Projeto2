CC = gcc
CFLAGS = -g -Wall -pthread 

all:
	$(CC) $(CFLAGS) river.c
