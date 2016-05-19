CC=gcc
CFLAGS= -ansi -std=c99
SOURCES=main.c
EXEC=minishell
minshell: main.o commands.o functions.o consts.h
	$(CC) -o $(EXEC) main.o commands.o functions.o $(CFLAGS)

main.o: commands.o functions.o main.c consts.h
	$(CC) -o main.o -c main.c  $(CFLAGS)

commands.o: functions.o commands.c commands.h consts.h
	$(CC) -o commands.o -c commands.c functions.o $(CFLAGS)

functions.o: functions.c functions.h consts.h
	$(CC) -o functions.o -c functions.c $(CFLAGS)

clean:
	rm -rf *.o

mrproper: clean
	rm -rf $(EXEC)


