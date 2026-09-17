/* This is an example of a program using the random server */
#include <mach.h>
#include <servers/netname.h>
#include "random.h"
#include <mach_error.h>
#include <stdio.h>


main()
{
    int		number;
    string25	password;
    port_t	serv_port;
    kern_return_t retcode;

	printf("Looking up port for random server\n");
	retcode = netname_look_up(name_server_port,"","RandomServerPort",
			&serv_port);
	if (retcode != KERN_SUCCESS)
	{    mach_error("error in looking up port for random server",retcode);
	     printf("Random server may not be running\n");
	     exit();
	}
	printf("Calling get_random\n");
	retcode = get_random(serv_port,&number);
	if (retcode == KERN_SUCCESS)
		printf("Result from get_random is %d\n",number);
	else mach_error("Error from get_random is",retcode);
	printf("Calling get_secret\n");
	retcode = get_secret(serv_port,password);
	if (retcode == KERN_SUCCESS)
		printf("Result from get_secret is %s\n",password);
	else mach_error("Error from get_secret is",retcode);
}
