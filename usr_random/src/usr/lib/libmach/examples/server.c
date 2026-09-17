/***************************************************
 *   Main program for random server
 **************************************************/

#include <stdio.h>
#include <mach.h>
#include <mach_error.h>
#include <mach/message.h>
#include <servers/netname.h>

extern boolean_t random_server();	/* from randomServer.c */
extern msg_return_t mig_server();	/* from mig_server.c */

main()
{
	port_name_t server_port;
	port_name_t port_set;
	kern_return_t kr;

	/* allocate a service port for receiving request messages */

	kr = port_allocate(task_self(), &server_port);
	if (kr != KERN_SUCCESS) {
		mach_error("port_allocate", kr);
		exit(1);
	}

	/* allocate a port set to hold the service port and notify port */

	kr = port_set_allocate(task_self(), &port_set);
	if (kr != KERN_SUCCESS) {
		mach_error("port_set_allocate", kr);
		exit(1);
	}

	/* put the service port and notify port into the port set */

	kr = port_set_add(task_self(), port_set, server_port);
	if (kr != KERN_SUCCESS) {
		mach_error("port_set_add", kr);
		exit(1);
	}

	kr = port_set_add(task_self(), port_set, task_notify());
	if (kr != KERN_SUCCESS) {
		mach_error("port_set_add", kr);
		exit(1);
	}

	/* check service port into the name service so clients can find it */

	kr = netname_check_in(name_server_port, "RandomServerPort",
			      PORT_NULL, server_port);
	if (kr != KERN_SUCCESS) {
		mach_error("netname_check_in", kr);
		exit(1);
	}

	/* call the standard service loop; should never return */

	kr = mig_server(port_set, random_server);
	mach_error("mig_server", kr);
	exit(2);
}
