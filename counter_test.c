#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "uthread.h"
#include "uthread_lock.h"

int shared_count = 0;
int shared_array[50] = { 0 };

mutex_t m;

int increment_and_print(void *arg)
{
	for (int i = 0; i < 10; ++i) {
		uthread_mutex_lock(m);

		shared_array[shared_count] = shared_count;

		int temp = shared_count;
		uthread_yield();
		shared_count = temp + 1;

		printf("%s, %d\n", (char *)arg, shared_count);
		uthread_mutex_unlock(m);
	}

	return 0;
}

int main(int argc, char **argv)
{
	int i;
	char *name = "Thread-X";
	char *names[5];

	for (i = 0; i < 5; ++i) {
		names[i] = malloc(9);
		strcpy(names[i], name);
		names[i][7] = i + '1';
	}

	uthread_mutex_create(&m);
	struct uthread_attr a = UT_ATTR(UT_DEFER, 0);

	for (i = 0; i < 5; ++i) {
		uthread_create(a, increment_and_print, names[i]);
	}

	uthread_joinall();
	uthread_mutex_destroy(m);

	int array_consistent = 1;
	for (i = 0; i < 50; ++i) {
		if (shared_array[i] != i) {
			array_consistent = 0;
			break;
		}
	}

	printf(array_consistent ? "results are consistent.\n"
	       : "results are inconsistent!\n");

	return 0;
}
