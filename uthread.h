#pragma once

#include <stdint.h>

#include <errno.h>

/* User level co-operatively scheduled threads API */

// thread IDs should be opaque
struct thread;
typedef struct thread *tid_t;

// errors
enum {
	UT_EINVAL = EINVAL,
	UT_EAGAIN = EAGAIN,
	UT_SO,
	UT_DEADLOCK,
};

// thread attribute literals
enum {
	UT_DEFER = 0x1,
	UT_PERM = 0x2,
	UT_JOIN = 0x4,
};

// thread attributes
struct uthread_attr {
	unsigned char DEFER:1;	// don't schedule immediately
	unsigned char PERM:1;	// don't free tcb after join
	unsigned char JOIN:1;	// this thread can be joined when living
	unsigned char jcount:6;	// number of times thread can be joined
};

// default attributes: DEFER, reclaim storage after 1 join
#define UT_DEF_ATTR (struct uthread_attr) {1,0,1}

#define UT_ATTR(flags, count) { \
    .DEFER = UT_DEFER&(flags) ? 1 : 0, \
    .PERM = UT_PERM&(flags) ? 1 : 0, \
    .JOIN = UT_JOIN&(flags) ? 1 : 0, \
    count }

typedef int (*uthread_inif) ();

/** yield
 *  Attempt to run another uthread, return when another uthread yields
 */
void uthread_yield(void);

/** create
 *  Create a new thread and schedule to run.
 *  @flags: Attributes that control thread creation and destrution behavior.
 *  @f: Function pointer entry point.
 *  @arg: Argument to thread. Caller is responsible for heap allocation and free.
 *
 *  Return value: The tid of the created thread or NULL if there was not enough
 *                memory to create a thread.
 */
tid_t uthread_create(struct uthread_attr flags, uthread_inif f, void *arg);

/** join
 *  Yield until a running thread exits and return its value in retval.
 *  Several things can happen here:
 *
 *  If tid is invalid (un-joinable, non-existent) then return error.
 *  If tid is valid, but it is attempting to join the calling thread, return error.
 *  Otherwise write the return value and exit with success (0).
 *
 *  Return value: 0 on success, UT_* on error.
 *
 *  @tid: Thread ID of running thread.
 *  @retval: If not NULL, the return value of finished thread will be here.
 */
int uthread_join(tid_t tid, int *retval);
int uthread_joinall(void);

struct uthread_attr uthread_getattr(void);

/** exit
 *  Terminate current thread and set return value as val.
 *  This function does not return.
 */
void uthread_exit(int val);

/** gettid
 *  Return the running thread's ID. This call should never fail (and return the NULL ID).
 */
tid_t uthread_gettid(void);

/** Plumbing API. This is an expirement conflating some things that probably should not be.
 *
 * OO API: f(...) => _f(current_tid, ...)
 */

void _uthread_yield(tid_t tid);
tid_t _uthread_gettid(tid_t tid);
void _uthread_exit(tid_t tid, int val);
tid_t _uthread_create(tid_t tid, struct uthread_attr flags, uthread_inif f,
		      void *arg);
struct uthread_attr _uthread_getattr(tid_t tid);

/* Not yet implemented ideas */

/** fork
 *  Duplucate running thread and schedule to run
 */
tid_t uthread_fork(struct uthread_attr flags);

#define uthread_createX(flags, f, ...) uthread_create2(flags, f, __VA_ARGS__, 0xAF)
int uthread_create2(unsigned int flags, uthread_inif f, ...);
int uthread_joinall();

// dumb debug function:
char *pr_tid(tid_t t);
