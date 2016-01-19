#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

#include <unistd.h>

#include "uthread.h"

/*
 * Make sure we don't crash when various API is called
 * w/o creating threads first.
 * 
 */
int main(int argc, char *argv[])
{
	int rc, rv;
	uthread_yield();
	tid_t mtid = uthread_gettid();

	printf("main tid: %p\n", mtid);
	uthread_yield();
	printf("joinall: %d\n", uthread_joinall());

	rc = uthread_join(mtid, &rv);
	printf("join(%p) returned: %d (%s)\n", mtid, rv, rc == 0 ? "success" : "failed");

	return rc;
}
