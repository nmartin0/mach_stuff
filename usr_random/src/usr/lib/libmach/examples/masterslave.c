/*
 * This program is an example of a master thread spawning a number of 
 * concurrent slaves.  The master thread waits until all of the slaves have
 * finished to exit.  Once created a slave process doesn't do much in this
 * simple example except loop.  A count variable is used by the master and
 * slave processes to keep track of the current number of slaves executing.
 * A mutex is associated with this count variable, and a condition variable
 * with the mutex.  This program is a simple demonstration of the use of
 * mutex and condition variables.
 */

#include <stdio.h>
#include <cthreads.h>

int count;         /* number of slaves active */
mutex_t lock;      /* mutual exclusion for count */
condition_t done;  /* signalled each time a slave finishes */

extern long random();

init()
{
    cthread_init();
    count = 0;
    lock = mutex_alloc();
    done = condition_alloc();
    srandom(time((int *) 0));  /* initialize random number generator */
}

/*
 * Each slave just counts up to its argument, yielding the processor on 
 * each iteration.  When it is finished, it decrements the global count
 * and signals that it is done.
 */
slave(n)
    int n;
{
    int i;

    for (i = 0; i < n; i += 1)
        cthread_yield();
    mutex_lock(lock);
    count -= 1;
    printf("Slave finished %d cycles.\n", n);
    condition_signal(done);
    mutex_unlock(lock);
}

/*
 * The master spawns a given number of slaves and then waits for them all to 
 * finish.
 */
master(nslaves)
    int nslaves;
{
    int i;

    for (i = 1; i <= nslaves; i += 1) {
        mutex_lock(lock);
        /*
         * Fork a slave and detach it,
         * since the master never joins it individually.
         */
        count += 1;
        cthread_detach(cthread_fork(slave, random() % 1000));
        mutex_unlock(lock);
    }
    mutex_lock(lock);
    while (count != 0)
        condition_wait(done, lock);
    mutex_unlock(lock);
    printf("All %d slaves have finished.\n", nslaves);
    cthread_exit(0);
}

main()
{
    init();
    master((int) random() % 16);  /* create up to 15 slaves */
}
