/* 
 * This program demonstrates the use of vm_inherit and copy on write
 * memory. A child and parent process will share memory, polling this 
 * memory to see whos turn it is to proceed.  First some memory is allocated,
 * and vm_inherit is called on this memory, the variable 'lock'.  Next more 
 * memory is allocated for the copy on write test. A fork is executed, and
 * The parent then stores new data in the copy on write memory
 * previously allocated, and sets the shared variable signaling to the
 * child that he is now waiting.  The child, polling the shared variable,
 * realizes it is his turn.  The child prints the value of the variable
 * lock and a value of the copy on write memory as the child sees it.
 * You will notice that the value of the lock is what the parent
 * set it to be, but the value of the copy on write memory is the original
 * value and not what the parent changed it to be.
 * The parent then awakes and prints out the two values once more.
 * The program then ends with the parent signaling the child via the
 * shared variable lock.
 ********************************************************/
#include <mach.h>
#include <stdio.h>

#define NO_ONE_WAIT 0
#define PARENT_WAIT 1
#define CHILD_WAIT 2
#define COPY_ON_WRITE 0
#define PARENT_CHANGED 1
#define CHILD_CHANGED 2

#define MAXDATA 100

main(argc, argv)
	int	argc;
	char	*argv[];
{
	int		pid;
	int		*mem;
	int		*lock;
	kern_return_t	ret;

	if (argc > 1) {
		printf("cowtest takes no switches.  ");
		printf("This program is an example of copy on write \n");
		printf("memory and of the use of vm_inherit.\n");
		exit();
	}
	if ((ret = vm_allocate(task_self(), &lock, sizeof(int),
		TRUE)) != KERN_SUCCESS) {
		mach_error("vm_allocate returned value of ", ret);
		printf("Exiting with error.\n");
		exit();
	}
	if ((ret = vm_inherit(task_self(), lock, sizeof(int),
		VM_INHERIT_SHARE)) != KERN_SUCCESS) {
		mach_error("vm_inherit returned value of ", ret);
		printf("Exiting with error.\n");
		exit();
	}
	*lock = NO_ONE_WAIT;
	if ((ret = vm_allocate(task_self(), &mem, sizeof(int) * MAXDATA,
		TRUE)) != KERN_SUCCESS) {
		mach_error("vm_allocate returned value of ", ret);
		printf("Exiting with error.\n");
		exit();
	}
	mem[0] = COPY_ON_WRITE;

	printf("value of lock before fork: %d\n", *lock);
	pid = fork();
	if (pid) {
		printf("PARENT: copied memory =  %d\n",
			mem[0]);
		printf("PARENT: changing to %d\n", PARENT_CHANGED);
		mem[0] = PARENT_CHANGED;
		printf("\n");
		printf("PARENT: lock = %d\n", *lock);
		printf("PARENT: changing lock to %d\n", PARENT_WAIT);
		printf("\n");
		*lock = PARENT_WAIT;
		while (*lock == PARENT_WAIT);
		   /* wait for child to change the value */
		   /* beware of optimizing compilers */
		printf("PARENT: copied memory = %d\n", 
			mem[0]);
		printf("PARENT: lock = %d\n", *lock);
		printf("PARENT: Finished.\n");
		*lock = PARENT_WAIT;
		exit();
	}
	while (*lock != PARENT_WAIT);
		/* wait for parent to change lock */ 
		/* beware of optimizing compilers */
	printf("CHILD: copied memory = %d\n", mem[0]);
	printf("CHILD: changing to %d\n", CHILD_CHANGED);
	mem[0] = CHILD_CHANGED;
	printf("\n");
	printf("CHILD: lock = %d\n", *lock);
	printf("CHILD: changing lock to %d\n", CHILD_WAIT);
	printf("\n");
	*lock = CHILD_WAIT;
	while (*lock == CHILD_WAIT);
		/* wait for parent to change lock */ 
	if ((ret = vm_deallocate(task_self(), lock, 
		sizeof(int), TRUE)) != KERN_SUCCESS) {
		mach_error("vm_deallocate returned value of ", ret);
		printf("Exiting.\n");
		exit();
	}
	if ((ret = vm_deallocate(task_self(), mem, 
		MAXDATA * sizeof(char), TRUE)) != KERN_SUCCESS) {
		mach_error("vm_deallocate returned value of ", ret);
		printf("Exiting.\n");
		exit();
	}
	printf("CHILD: Finished.\n");
}
