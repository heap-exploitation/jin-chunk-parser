CC = gcc
CFLAGS = -g -O0 -Wall -Werror
LDFLAGS = -lc -ldl

SRC = dump_chunk.c

all:
	$(CC) $(CFLAGS) $(SRC) $(LDFLAGS)

clean:
	rm -f a.out
