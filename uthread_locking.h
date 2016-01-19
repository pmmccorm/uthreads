#ifndef UTHREAD_LOCKING
#define UTHREAD_LOCKING

#include <stdatomic.h>
#include <stdbool.h>

typedef atomic_flag spinlock_t;

#define UT_SPINLOCK_UNLOCKED ATOMIC_FLAG_INIT

static inline
void uthread_spinlock(spinlock_t *sl) {
	while (atomic_flag_test_and_set(sl) == true) {}
}

static inline
void uthread_spinunlock(spinlock_t *sl) {
	atomic_flag_clear(sl);
}

#include <stdarg.h>
static inline
void PRINTF(const char *fmt, ...)
{
	static spinlock_t l = UT_SPINLOCK_UNLOCKED;
	va_list argp;
	va_start(argp, fmt);

	uthread_spinlock(&l);
	vprintf(fmt, argp);
	uthread_spinunlock(&l);
	va_end(argp);
}

#endif
