#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

#include <unistd.h>

#include "uthread.h"
#include "uthread_locking.h"

int factorial(int n)
{
	return n == 0 ? 1 : n * factorial(n - 1);
}

int fun_with_threads(void *arg)
{
	int n = *(int *)arg;
	struct uthread_attr a = uthread_getattr();

	PRINTF("  %s\n", pr_tid(uthread_gettid()));

	PRINTF("  RESULT: %d\n", factorial(n));

	sleep(10);
	return n;
}

/*
 * Allocate TCB for 1 new thread (with stack, init argument)
 * and TCB for inactive thread
 */
int main(int argc, char *argv[])
{
	int rc, rv, i = atoi(argv[1]);
	void *last_tid;

	printf("main tid: %p\n", uthread_gettid());

	for (; i > 0; --i) {
		last_tid = uthread_create(UT_DEF_ATTR, fun_with_threads, &i);
		assert(last_tid != NULL);
		printf("queued thread: %p\n", last_tid);
	}

	rc = uthread_joinall();
	printf("joinall() returned: %s \n", rc == 0 ? "success" : "failed");

	return rc;
}
