CC=gcc
CFLAGS= -ansi -std=c99 -Wall
SOURCES=main.c
EXEC=minishell
all:
	$(CC) $(CFLAGS) -o $(EXEC) $(SOURCES) 

mrproper: clean
	rm -rf minishell
