#include <stdio.h>
#include <stdlib.h>

#include "uthread.h"

int join_thread(tid_t tid)
{
	int rv, rc;
	tid_t mtid = uthread_gettid();
	rc = uthread_join(tid, &rv);

	printf("  %p joins %p (%d)\n", mtid, tid, rv);

	if (rc)
		return rc;
	else
		return rv;
}

int fun_with_threads(void *arg)
{
	struct uthread_attr *a = arg;
	struct uthread_attr na = UT_ATTR(UT_DEFER, 0);
	int c = a->jcount;
	tid_t last_tid, tid = uthread_gettid();

	for (int i = 0; i < a->jcount; ++i) {
		last_tid = uthread_create(na, join_thread, &tid);
		printf("  %p created thread: %p\n", tid, last_tid);
	}

	return c;
}

/*
 * Test some properties of defer and exit 
 *
 */
int main(int argc, char *argv[])
{
	int rc, rv, n;
	void *last_tid;
	tid_t tid = uthread_gettid();

	if (argc <= 1)
		n = 3;
	else
		n = atoi(argv[1]);

	printf("%s\n", pr_tid(tid));

	for (int i = n; i > 0; --i) {
		struct uthread_attr attrs = UT_ATTR(0, i);
		last_tid = uthread_create(attrs, fun_with_threads, &attrs);
		printf("%p created thread: %p\n", tid, last_tid);
	}

	rc = uthread_joinall();
	printf("joinall(): %d\n", rc);

	return rc;
}
