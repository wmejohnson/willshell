CC = gccx
OPTIONS = -g -lreadline

default: willshell.c
	$(CC) $(OPTIONS) willshell.c -o willshell

test: test.c
	$(CC) $(OPTIONS) test.c -o test

clean:
	rm core
