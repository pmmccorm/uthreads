#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

#include "uthread.h"

struct uthread_attr def_attrs = UT_ATTR(0, 0);

int factorial(void *arg)
{
	int n = *(int *)arg;
	int m = n-1;
	return n == 0 ? 1 : n * factorial(&m);
}

int fun_with_threads(void *arg)
{
	int m = *(int *)arg;

	for (int i = 0; i < m; ++i) {
		int *ip = malloc(sizeof(*ip));
		*ip = i;
		printf("  %p: create %p\n", uthread_gettid(),
		       uthread_create(def_attrs, factorial, ip));
	}

	return m;
}

/*
 * Create N threads which spawn M threads (default 1x1)
 */
int main(int argc, char *argv[])
{
	int n, m;
	int rc;
	void *prev_tid = NULL, *last_tid = NULL;

	if (argc < 3)
		n = m = 1;
	else {
		n = atoi(argv[1]);
		m = atoi(argv[2]);
	}

	for (int i = 0; i < n; ++i) {
		last_tid = uthread_create(def_attrs, fun_with_threads, &m);

		if (last_tid) {
			printf("created thread: %p\n", last_tid);
			prev_tid = last_tid;
		} else {
			printf("out of memory, exiting\n");
			break;
		}
	}

	rc = uthread_joinall();
	printf("join returned: %d\n", rc);

	return EXIT_SUCCESS;
}
