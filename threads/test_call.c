#include <cthreads.h>
#include <mach.h>
#include <servers/machid.h>
#include <servers/machid_lib.h>

#define  mach_privileged_host_port() task_by_pid(-1)

void main(int argc, char **argv)
{
    vm_address_t addr;
    kern_return_t ret;
    int mid;
    task_t task;
    mach_port_t mid_server;
    mach_port_t mid_auth;
    cthread_t *thread_list;
    int thread_count;
    int i;

    if (argc != 3) {
	printf("usage: test_call mid address\n");
	exit(1);
    }

    mid = atoi(argv[1]);
    addr = atoi(argv[2]);
    ret = netname_look_up(name_server_port, "", "MachID", &mid_server);
    if (ret != KERN_SUCCESS) {
	mid_server = MACH_PORT_NULL;
      
	printf("Unable to find MachId server: %s.", mach_error_string (ret));
	exit(1);
    }
  
    mid_auth = mach_privileged_host_port();
    if (mid_auth == MACH_PORT_NULL)
	mid_auth = mach_task_self();
  
    ret = machid_mach_port (mid_server, mid_auth, mid, &task);
      if (ret != KERN_SUCCESS) {
	printf("Unable to map MID %d to task port: %s", mid,
	       mach_error_string (ret));
	exit(1);
    }

    ret = cthread_init_thread_calls(task, addr);
    if (ret != KERN_SUCCESS) {
	printf("cthread_init_thread_calls: %s\n", mach_error_string (ret));
	exit(1);
    }

    ret = cthread_threads(task, &thread_list, &thread_count);
    if (ret != KERN_SUCCESS) {
	printf("cthread_threads: %s\n", mach_error_string (ret));
	exit(1);
    }

    printf("%d threads\n",thread_count);
    for(i=0;i<thread_count;i++) {
	thread_state state;
	cthread_info_t info;
	int count;
	printf("%8x",(int)thread_list[i]);
	count = STATE_COUNT;
	ret = cthread_get_state(thread_list[i], STATE_FLAVOR, 
				&state, &count);
	if (ret == KERN_SUCCESS) {
	    ret = cthread_info(thread_list[i], &info);
	    if (ret == KERN_SUCCESS) {
		printf("[%8x] %16s %c%c%c eip %8x uesp %8x ebp %8x\n",
		       (int)info.remote_cthread,
		       (info.name?info.name:"None"),
		       (info.running?'R':'B'),
		       (info.runnable?'R':' '),
		       (info.waiter?'W':' '),
		       state.eip, 
		       state.uesp, state.ebp);
	    }
	} else
	    printf(" %s\n", mach_error_string (ret));
    }
}
