CFLAGS?=-std=c11 -g -DDEBUG -D_GNU_SOURCE
CPPFLAGS+=-I. -Iccan/list/

.DEFAULT: tests

.PHONY: tests
TESTS:=example NxM join fp assign3 snake null counter_test sort_test
tests: $(TESTS)

OBJS := uthread.o asm.o list.o locking.o

example: example.c $(OBJS)
NxM: NxM.c $(OBJS)
join: join.c $(OBJS)
fp: fp.c $(OBJS)
null: null.c $(OBJS)
counter_test: counter_test.c $(OBJS)
sort_test: sort_test.c $(OBJS)

snake: snake.c $(OBJS)
	$(CC) $(CFLAGS) $(CPPFLAGS) $^ -o $@ -lrt
assign3: assign3.c $(OBJS)
	$(CC) $(CFLAGS) $(CPPFLAGS) $^ -o $@ -lrt

uthread.o: uthread.h uthread.c locking.o asm.o list.o

asm.o: asm.s

list.o: list.c

locking.o: uthread_lock.h locking.c

.PHONY: clean
clean:
	rm -f *.o
	rm $(TESTS)
