#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>

#include <sched.h>

#include "ccan/list/list.h"

#include "uthread.h"
#include "uthread_locking.h"

// prototypes from asm.s
extern void thread_switch(struct thread *old, struct thread *new, spinlock_t *lock);
extern void thread_start(struct thread *old, struct thread *new, spinlock_t *lock);

#define _CANARY UINT64_C(0xABCDEF)
#define _UTHREAD_STACKSZ 4096
static const uint64_t CANARY = _CANARY;
static const size_t UTHREAD_STACKSZ = _UTHREAD_STACKSZ;

// overloading state to hold tid of join
// TODO: we may not need these now w/ pool lists
enum thread_state {
	READY = 0u,
	RUNNING,
	RECLAIM,
	DEAD,
};

struct thread {
	unsigned char *stack_pointer;
	struct list_node list;
	enum thread_state state;
	int retval;
	struct uthread_attr attr;
	uthread_inif init_function;
	void *initial_argument;
	struct list_head blocked;
	unsigned char stack_top[];
};

static struct thread_pool {
	int count;
	int max;
	struct list_head running;
	struct list_head joinable;
	struct list_head freelist;
	spinlock_t lock;
	bool init;
} default_pool = {
	.count = 1,
	.max = 2,
	.lock = UT_SPINLOCK_UNLOCKED,
	.init = false,
};

// TCB for main thread (PERM|DETACH)
static struct thread uthread_main = {
	.state = RUNNING,
	.attr = UT_ATTR(UT_PERM, 0),
};

static _Thread_local struct thread *uthread_current = NULL;

static inline
struct thread *toTCB(struct list_node *l)
{
	return container_of(l, struct thread, list);
}

static inline
void list_move(struct list_head *to, struct list_node *node)
{
	uthread_spinlock(&default_pool.lock);
	list_del(node);
	list_add(to, node);
	uthread_spinunlock(&default_pool.lock);
}

static inline
void list_move_nolock(struct list_head *to, struct list_node *node)
{
	list_del(node);
	list_add(to, node);
}

void pr_list(struct list_head *ls)
{
	struct thread *next;
	int n = 0;

	list_for_each(ls, next, list) {
		++n;
		PRINTF("DEBUG: %s\n", pr_tid(next));
	}
	PRINTF("DEBUG: size: %d, count: %d\n", n, default_pool.count);
}

/*
 * We need to add the main thread to list of running threads at
 * some point, put this here for convenience
 * TODO: make a run_once() function for this
 * TODO: do we need locking?
 * IDEA: init the pool locked, have init unlock it
 */
static void init()
{
	list_head_init(&default_pool.running);
	list_head_init(&default_pool.joinable);
	list_head_init(&default_pool.freelist);

	uthread_current = &uthread_main;
	list_add(&default_pool.running, &uthread_current->list);
	list_head_init(&uthread_main.blocked);
	default_pool.init = true;
}

struct uthread_attr _uthread_getattr(struct thread *tcb)
{
	return tcb->attr;
}

struct uthread_attr uthread_getattr(void)
{
	if (!default_pool.init)
		init();

	return uthread_current->attr;
}

tid_t uthread_gettid()
{
	if (!default_pool.init)
		init();

	return uthread_current;
}

// TODO: think about PERM more
// TODO: we don't want uthread_current to not be on the running list for yield()
// TODO: threads with jcount = 0 should also dump their blocked onto run list
void _uthread_exit(tid_t tid, int val)
{
	struct thread *tcb = tid;

	// a PERM thread tries to exit?
	if (tcb->attr.PERM)
		return;

	uthread_spinlock(&default_pool.lock);
	--default_pool.count;

	if (tcb->attr.jcount == 0) {
		tcb->state = RECLAIM;
		list_append_list(&default_pool.running, &tcb->blocked);
		list_move_nolock(&default_pool.freelist, &tcb->list);
	} else {
		tcb->state = DEAD;
		tcb->retval = val;
		list_move_nolock(&default_pool.joinable, &tcb->list);
		list_append_list(&default_pool.running, &tcb->blocked);
	}

	uthread_spinunlock(&default_pool.lock);

	uthread_yield();
}

void uthread_exit(int val)
{
	_uthread_exit(uthread_current, val);
}

void _uthread_yield(struct thread *next)
{
	struct thread *cur = uthread_current;

	uthread_spinlock(&default_pool.lock);
	uthread_current = next;
	next->state = RUNNING;

	if (next->attr.DEFER) {
		cur->state = READY;
		next->attr.DEFER = 0;
		thread_start(cur, next, &default_pool.lock);
	} else {
		cur->state = READY;
		thread_switch(cur, next, &default_pool.lock);
	}
}

// TODO: I have not thought very hard about this.
// Note: We can switch to a waiting thread. This must mean that yield was
// called from join.
void uthread_yield()
{
	struct thread *next, *found = NULL;

	if (!default_pool.init)
		init();

#if DEBUG == 1
	list_check(&default_pool.running, "list failed check");
	list_check_node(&uthread_current->list, "list failed check");
#endif

	uthread_spinlock(&default_pool.lock);

	list_for_each(&default_pool.running, next, list) {
		if (next->state == READY) {
			found = next;
			found->state = RUNNING;
			break;
		}
	}
	uthread_spinunlock(&default_pool.lock);

	if (found)
		_uthread_yield(found);
}

// TODO: locking
int uthread_joinall(void)
{
	while (1) {
		struct thread *ptr, *found = NULL;

		uthread_spinlock(&default_pool.lock);
		pr_list(&default_pool.running);

		if (list_prev(&default_pool.running, uthread_current, list) ==
			list_next(&default_pool.running, uthread_current, list)) {
			uthread_spinunlock(&default_pool.lock);
			return EXIT_SUCCESS;
		} else {
			uthread_spinunlock(&default_pool.lock);
			uthread_yield();
		}
	};
}

// TODO: deadlock detection
int uthread_join(tid_t tid, int *retval)
{
	int rc;
	struct thread *target = tid;

	// can't wait on itself to finish
	if (tid == uthread_current)
		return -UT_EINVAL;

	// trying to join a permananet thread
	if (target->attr.PERM == 1)
		return -UT_EINVAL;

	// just return retval, this thread is dead
	if (target->attr.JOIN == 1 && target->attr.jcount > 0) {
		if (retval)
			*retval = target->retval;
		--target->attr.jcount;
		return EXIT_SUCCESS;
	}

	uthread_spinlock(&default_pool.lock);

	// trying to join a live thread, block on it
	// IDEA: if the thread is READY we could just yield to it directly
	if ((target->state == RUNNING || target->state == READY) &&
		target->attr.jcount > 0) {
		list_move_nolock(&target->blocked, &uthread_current->list);
		uthread_spinunlock(&default_pool.lock);
		uthread_yield();
		uthread_spinlock(&default_pool.lock);
	}

	if (target->state == DEAD && target->attr.jcount > 0) {
		--target->attr.jcount;
		if (retval)
			*retval = target->retval;

		if (target->attr.jcount == 0) {
			target->state = RECLAIM;
			list_move_nolock(&default_pool.freelist, &target->list);
		}

		uthread_spinunlock(&default_pool.lock);
		return EXIT_SUCCESS;
	}

	// thread is RECLAIMED?
	uthread_spinunlock(&default_pool.lock);
	return -UT_EINVAL;
}

void uthread_wrap()
{
	struct thread *cur = uthread_current;

	PRINTF("XXX: %p\n", uthread_current);
	_uthread_exit(cur, cur->init_function(cur->initial_argument));
}

int clone_wrap(void *tcb)
{
	uthread_current = tcb;
	uthread_current->retval = 
	uthread_current->init_function(uthread_current->initial_argument);

	PRINTF("YYY: %p\n", uthread_current);
	if (uthread_current->attr.PERM == 0 && uthread_current->attr.jcount > 0)
		uthread_current->attr.JOIN = 1;
	
	--default_pool.count;
	while (default_pool.count > default_pool.max) {
		PRINTF(".");
		uthread_yield();
	}
	PRINTF("+");

	//now clean up..
	uthread_spinlock(&default_pool.lock);

	if (uthread_current->attr.jcount == 0) {
		uthread_current->state = RECLAIM;
		uthread_current->attr.JOIN = 0;
		list_append_list(&default_pool.running, &uthread_current->blocked);
		list_move_nolock(&default_pool.freelist, &uthread_current->list);
	} else {
		uthread_current->state = DEAD;
		list_append_list(&default_pool.running, &uthread_current->blocked);
		list_move_nolock(&default_pool.joinable, &uthread_current->list);
	}

	uthread_spinunlock(&default_pool.lock);

	PRINTF("ZZZ: %p\n", uthread_current);
	return 0;
}

// try and grab a RECLAIMed thread vs always malloc
// TODO: don't assume all stacks are same size, alloc tcb + stack seperate
static struct thread *pmalloc(size_t size)
{
	uthread_spinlock(&default_pool.lock);
	struct thread *new_thread = list_pop(&default_pool.freelist, struct thread, list);
	PRINTF("RETURNING: %p\n", new_thread);
	uthread_spinunlock(&default_pool.lock);

	if (new_thread)
		return new_thread;
	else
		return malloc(sizeof(*new_thread) + UTHREAD_STACKSZ);
}

// TODO: think out thread attrs and deal with them properly
tid_t uthread_create(struct uthread_attr flags, uthread_inif inif, void *arg)
{
	struct thread *new_thread, *cur_thread;
	int rc;

	if (!default_pool.init)
		init();

	cur_thread = uthread_current;

	new_thread = pmalloc(0);

	if (!new_thread)
		return NULL;

	new_thread->stack_pointer = new_thread->stack_top + UTHREAD_STACKSZ;

	new_thread->init_function = inif;
	new_thread->initial_argument = arg;
	new_thread->attr = flags;
	list_head_init(&new_thread->blocked);

	uthread_spinlock(&default_pool.lock);

	list_add(&default_pool.running, &new_thread->list);

	// call clone, return tid or 0
	if (default_pool.max == 0 ||
		default_pool.count < default_pool.max) {
		new_thread->state = RUNNING;
		new_thread->attr.DEFER = 0;
		rc = clone(clone_wrap, new_thread->stack_pointer,
				CLONE_THREAD | CLONE_VM | CLONE_SIGHAND | CLONE_FILES | CLONE_FS | CLONE_IO,
				new_thread);
		
		// TODO: remove new thread on failure
		if (rc == -1)
			return NULL;
		else
			++default_pool.count;

		uthread_spinunlock(&default_pool.lock);
	} else if (flags.DEFER) {
		// just return tid
		new_thread->state = READY;
		++default_pool.count;
		uthread_spinunlock(&default_pool.lock);

	} else {
		// switch in current thread
		++default_pool.count;
		uthread_current->state = READY;
		uthread_current = new_thread;
		uthread_current->state = RUNNING;
		thread_start(cur_thread, new_thread, &default_pool.lock);
	}

	return new_thread;
}

char *pr_tid(tid_t t)
{
	struct uthread_attr a = _uthread_getattr(t);
	struct thread *tcb = t;
	static char buf[64] = { 0 };

	char *st[] = {[READY] = "READY",
				  [RUNNING] = "RUNNING",
				  [DEAD] = "DEAD",
				  [RECLAIM] = "RECLAIM",
				 };

	if (a.DEFER && a.PERM)
		sprintf(buf, "%p(UT_DEFER|UT_PERM),%d: %s", t, a.jcount, st[tcb->state]);
	else if (a.DEFER)
		sprintf(buf, "%p(UT_DEFER),%d: %s", t, a.jcount, st[tcb->state]);
	else if (a.PERM)
		sprintf(buf, "%p(UT_PERM),%d: %s", t, a.jcount, st[tcb->state]);
	else
		sprintf(buf, "%p(),%d: %s", t, a.jcount, st[tcb->state]);

	return buf;
}
