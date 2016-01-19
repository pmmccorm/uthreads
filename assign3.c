#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

#include <unistd.h>
#include <aio.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "uthread.h"
#include "uthread_aio.h"

int factorial(int n)
{
	return n == 0 ? 1 : n * factorial(n - 1);
}

int fun_with_threads(void *arg)
{
	int n = *(int *)arg;
	struct uthread_attr a = uthread_getattr();

	printf("  %s\n", pr_tid(uthread_gettid()));

	for (int i = n; i > 0; --i) {
		printf("  %d ", factorial(i));
	}

	printf("\n");
	return n;
}

/*
 * 
 *
 */
int main(int argc, char *argv[])
{
	int fd, rv;
	void *last_tid;
	char buf[4096];

	printf("main tid: %p\n", uthread_gettid());
	struct uthread_attr a = UT_DEF_ATTR;

	for (int i = 5; i > 0; --i) {
		last_tid = uthread_create(a, fun_with_threads, &i);
		printf("queued thread: %p\n", last_tid);
	}

	fd = open(argv[1], O_RDONLY);
	rv = async_read(fd, buf, 4096);
	printf("async_read (%d): %s\n", rv, buf);

	rv = uthread_joinall();
	printf("join returned: %d\n", rv);

	return EXIT_SUCCESS;
}
