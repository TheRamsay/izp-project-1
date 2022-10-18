CC=gcc
CFLAGS = -std=c99 -Wall -Wextra -Werror -ggdb

t9search: t9search.o
	$(CC) -o t9search t9search.c

