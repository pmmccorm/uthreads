#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include "ccan/list/list.h"

#include "uthread.h"
#include "uthread_lock.h"

struct _condrec {
	tid_t tid;
	struct list_node list;
};

struct uthread_cond {
	struct list_head waiting;
};

struct uthread_mt {
	int held;
	tid_t by;
};

bool list_contains(struct list_head *head, struct list_node *node)
{
	return true;
}

int uthread_cond_create(cond_t *cond)
{
	struct uthread_cond *tmp = malloc(sizeof(*tmp));

	assert(tmp);

	list_head_init(&tmp->waiting);
	*cond = tmp;
	return 0;
}

// init unlocked
int uthread_mutex_create(mutex_t *mt)
{
	struct uthread_mt *tmp = malloc(sizeof(*tmp));

	assert(tmp);

	tmp->held = 0;
	tmp->by = 0;
	*mt = tmp;
	return 0;
}

void uthread_mutex_destroy(mutex_t mt)
{
	assert(mt->held == 0);
	free (mt);
}
int uthread_mutex_lock(mutex_t mt)
{
	tid_t my_tid = uthread_gettid();

	assert(!(mt->held && mt->by == my_tid));

	while (mt->held)
		uthread_yield();
	
	mt->held = 1;
	mt->by = my_tid;

	return 0;
}

int uthread_mutex_unlock(mutex_t mt)
{
	assert(mt->held != 0);
	assert(mt->by == uthread_gettid());

	mt->held = 0;
	return 0;
}

int uthread_cond_wait(cond_t cond, mutex_t mt)
{
	struct _condrec cr = { .tid = uthread_gettid() };

	if (!mt->held)
		return -UT_EINVAL;
	
	if (uthread_mutex_unlock(mt))
		return -UT_EINVAL;

	list_add(&cond->waiting, &cr.list);

	do {
		uthread_yield();
	} while (list_contains(&cond->waiting, &cr.list));

	return uthread_mutex_lock(mt);
}

int uthread_cond_signal(cond_t cond)
{
	struct _condrec *cr = list_pop(&cond->waiting, struct _condrec, list);

	if (!cr || cr->tid == NULL)
		return -UT_EINVAL;

	_uthread_yield(cr->tid);

	return 0;
}
