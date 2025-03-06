CC = gcc
CFLAGS = -g -O0 -Wall -Werror

SRC = maybe_bug.c

all:
	$(CC) $(CFLAGS) $(SRC) $(LDFLAGS)

clean:
	rm -f a.out
