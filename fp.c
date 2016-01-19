#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

#include <unistd.h>

#include "uthread.h"

int fun_with_threads(void *arg)
{
	int n = *(int *)arg;
	struct uthread_attr a = uthread_getattr();
	float f1 = 1.0f, f2 = (float)n;

	printf("  %s\n", pr_tid(uthread_gettid()));

	printf("  %p: %f * 1.0 == %f\n", uthread_gettid(), f2, f1 * f2);
	uthread_yield();
	printf("  %p: %f * 1.0 == %f\n", uthread_gettid(), f2, f1 * f2);

	printf("\n");
	return n;
}

/*
 * Allocate TCB for 1 new thread (with stack, init argument)
 * and TCB for inactive thread
 */
int main(int argc, char *argv[])
{
	int rc, rv;
	void *last_tid;

	printf("main tid: %p\n", uthread_gettid());

	for (int i = 5; i > 0; --i) {
		float f1 = 1.0, f2 = 2.0, f3 = 3.0;
		last_tid = uthread_create(UT_DEF_ATTR, fun_with_threads, &i);
		printf("started thread: %p\n", last_tid);
	}

	rc = uthread_joinall();
	printf("join returned: %d\n", rc);

	return EXIT_SUCCESS;
}
