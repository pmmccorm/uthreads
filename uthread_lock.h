#include <assert.h>
#include <stdio.h>

#include "uthread.h"

// lock types are opaque pointers
struct uthread_cond;
typedef struct uthread_cond *cond_t;

struct uthread_mt;
typedef struct uthread_mt *mutex_t;

int uthread_cond_create(cond_t *cond);
void uthread_cond_destroy(cond_t cond);

int uthread_mutex_create(mutex_t *mt);
void uthread_mutex_destroy(mutex_t mt);

int uthread_mutex_lock(mutex_t mt);
int uthread_mutex_unlock(mutex_t mt);
int uthread_cond_wait(cond_t cond, mutex_t mt);
int uthread_cond_signal(cond_t cond);

