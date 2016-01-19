#include <unistd.h>
#include <stdio.h>
#include <stdarg.h>

#include "uthread_locking.h"

_Thread_local static int x = 90;

struct bf {
    unsigned char a : 1;
    unsigned char b : 1;
    unsigned char   : 6;
};    

_Thread_local static struct s {
    int x;
    int z;
} s_test;

int f1(struct s i)
{
    return i.x + i.z;
}

int g1(char c, char b, int x)
{
    return x;
}

int f2(float ff, ...)
{
    va_list ap;
    unsigned long int ptr;

    va_start(ap, ff);
    for (; ptr != 99; ptr = va_arg(ap, unsigned long int))
	printf("%lu\n", ptr);

    va_end(ap);
   
    return 0;
}

extern void unlock(spinlock_t *sp);

int main (int argc, char *argv[])
{
    int i;
	spinlock_t lock = UT_SPINLOCK_UNLOCKED;
    printf("%p\n", &*__environ);
    printf("%lu\n", sizeof(lock));

    i = f2(1,2,3,4,5,6,99);
    i = g1('v', 'g', argc);
    i = f1((struct s){1,2});

	uthread_spinlock(&lock);
	unlock(&lock);

//	printf("%d\n", lock);

    return  0;
}
