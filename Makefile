CFLAGS=-Wall -std=c11 -O -g
LDFLAGS=-g -static

all: test

bench:  test.c context_posix.o context_fast.o context_fastest.o switch_context_fastest.o
	$(CC) -DBENCH $(LDFLAGS) $^ -o $@
	./bench
	./bench

test: test.o context_posix.o context_fast.o context_fastest.o switch_context_fastest.o
	$(CC) $(LDFLAGS) $^ -o $@

test.o: test.c

context_posix.o: context_posix.c

context_fast.o: context_fast.c

switch_context_fastest.o: switch_context_fastest.S
