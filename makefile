CC = gccx
OPTIONS = -g -lreadline

default: willshell.c
	$(CC) $(OPTIONS) willshell.c -o willshell

clean:
	rm core
