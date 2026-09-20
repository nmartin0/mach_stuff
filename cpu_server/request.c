/*
 * Mach Operating System
 * Copyright (c) 1991,1990 Carnegie Mellon University
 * All Rights Reserved.
 * 
 * Permission to use, copy, modify and distribute this software and its
 * documentation is hereby granted, provided that both the copyright
 * notice and this permission notice appear in all copies of the
 * software, derivative works or modified versions, and any portions
 * thereof, and that both notices appear in supporting documentation.
 * 
 * CARNEGIE MELLON ALLOWS FREE USE OF THIS SOFTWARE IN ITS 
 * CONDITION.  CARNEGIE MELLON DISCLAIMS ANY LIABILITY OF ANY KIND FOR
 * ANY DAMAGES WHATSOEVER RESULTING FROM THE USE OF THIS SOFTWARE.
 * 
 * Carnegie Mellon requests users of this software to return to
 * 
 *  Software Distribution Coordinator  or  Software.Distribution@CS.CMU.EDU
 *  School of Computer Science
 *  Carnegie Mellon University
 *  Pittsburgh PA 15213-3890
 * 
 * any improvements or extensions that they make and grant Carnegie the
 * rights to redistribute these changes.
 */
/*
 * HISTORY
 * $Log:	request.c,v $
 * Revision 2.3  91/02/08  16:05:28  mrt
 * 	Added new Mach copyright and author notices
 * 
 * Revision 2.2  90/01/24  23:33:47  mrt
 * 	Changed include of sys/error.h to mach/error.h
 * 	[90/01/22            mrt]
 * 
 * 	Created
 * 	[90/01/18            dlb]
 * 
 */
/*
 *	File: 	request.c
 *	Author:	David Black, Carnegie Mellon University
 *	Date:	Jan 1990
 *
 *	Request management
 */
#include <mach.h>
#include <cthreads.h>
#include <mach/error.h>
#include <sys/time.h>

#include <stdio.h>

#include "request.h"

/*
 *	Initialize this module.
 */

void
request_init()
{
    queue_init(&all_requests);
    mutex_init(&all_requests_lock);
    queue_init(&free_requests);
    mutex_init(&free_requests_lock);
    queue_init(&time_queue);
    mutex_init(&time_queue_lock);
    condition_init(&time_queue_condition);
}

/*
 *	Allocate a request.
 */

request_t
request_alloc()
{
    request_t	new_request;
    
    /*
     *	Allocate the data structure.
     */
    mutex_lock(&free_requests_lock);
    if (queue_empty(&free_requests)) {
	new_request = (request_t) malloc(sizeof(struct request));
    }
    else {
	new_request = queue_first(&free_requests);
	queue_remove(&free_requests, new_request, request_t, all_req);
    }
    mutex_unlock(&free_requests_lock);
    
    /*
     *	Initialize the data structure.
     */
    bzero(new_request, sizeof(*new_request));
    port_allocate(task_self(), &new_request->port);
    new_request->state = REQUEST_INACTIVE;
    queue_init(&(new_request->all_req));
    queue_init(&(new_request->active_req));
    mutex_init(&new_request->lock);
    new_request->ref_count = 1;
    new_request->end_notify_done = FALSE;
    new_request->resume_needed = FALSE;

    /*
     *	Insert in all requests list.
     */
    mutex_lock(&all_requests_lock);
    queue_enter(&all_requests, new_request, request_t, all_req);
    mutex_unlock(&all_requests_lock);
    return(new_request);
}

/*
 *	Deallocate request -- free if last reference gone.
 */
request_deallocate(request)
request_t	request;
{
    request_t	prev_request;
    
    request_lock(request);
    if (--(request->ref_count) != 0) {
	request_unlock(request);
    	return;
    }
    /*
     *	This is the last reference, but the convert routine may make
     *	more from the all_requests list.  So put back one reference
     *	and grab the locks in the right order.
     */
    request->ref_count = 1;
    request_unlock(request);

    mutex_lock(&all_requests_lock);
    request_lock(request);
    if (--(request->ref_count) > 0) {
	request_unlock(request);
	mutex_unlock(&all_requests_lock);
	return;
    }

    queue_remove(&all_requests, request,request_t, all_req);
    mutex_unlock(&all_requests_lock);
    request_unlock(request);

    port_set_remove(task_self(), request->port);
    port_deallocate(task_self(), request->port);
    if (request->options & CPU_OPTION_DESTROY)
    	destroy_sets(request);
    
    mutex_lock(&free_requests_lock);
    queue_enter(&free_requests, request, request_t, all_req);
    mutex_unlock(&free_requests_lock);
}

/*
 *	Given a port, convert to a request.  Returns REQUEST_NULL if not
 *	found.  Uses linear list because small number of requests are
 *	expected.
 */

request_t
convert_port_to_request(port)
cpu_request_t	port;
{
    request_t	request;

    mutex_lock(&all_requests_lock);
    request = queue_first(&all_requests);
    while (!queue_end(&all_requests, request)) {
	if (request->port == port && request->state != REQUEST_DEAD) {
	    request_lock(request);
	    request->ref_count++;
	    request_unlock(request);
	    mutex_unlock(&all_requests_lock);
	    return(request);
	}
	request = request->all_req.next;
    }
    mutex_unlock(&all_requests_lock);
    return(REQUEST_NULL);
}

/*
 *	current_time() - return the current time in tenths of seconds
 */
int current_time()
{
    struct timeval	tv;
    struct timezone	tz;

    gettimeofday(&tv,&tz);
    return(tv.tv_sec*10 + tv.tv_usec/100000);
}

/*
 *	request_delay()	- calculate delay on current time queue until
 *		specified request can be honored.  Queue must be locked.
 *		returned delay is in seconds for use by clients.
 */
int request_delay(request)
    request_t	request;
{
    int assigned_delay = 0;
    int activated_delay = 0;
    int this_delay;
    request_t	tmp_request;

    tmp_request = queue_first(&time_queue);

    while (!queue_end(&time_queue, tmp_request) && tmp_request != request) {
    	switch(tmp_request->state) {
	    case REQUEST_ACTIVATED:
	    	activated_delay += tmp_request->run_time;
	    	break;
	    case REQUEST_ASSIGNED:
	    	this_delay = tmp_request->event_time - current_time();
		if (this_delay > assigned_delay)
		    assigned_delay = this_delay;
		break;
	    default:
	    	fprintf(stderr, "Bad request state.\n");
	    	exit(1);
	}
	tmp_request = tmp_request->active_req.next;
    }

    return((assigned_delay + activated_delay)/10);
}

/*
 *	Entry point implementations start here.
 */

mach_error_t
cpu_request_create(server, total_processors, time, delay, cpu_request)
cpu_request_t	server, *cpu_request;
int	total_processors, time, *delay;
{
    request_t	request;

    /*
     *	Argument checks.
     */
    if (server != server_port ||
	(total_processors < MIN_PROCESSORS) ||
        (total_processors > max_processors) ||
	(time < MIN_TIME) ||
	(time > max_time)) {
	    cpu_request = PORT_NULL;
	    return(CPU_INVALID_ARGUMENT);
    }
    request = request_alloc();
    request->total_processors = total_processors;
    request->run_time = time * 10;
    request->total_time = request->run_time;
    *cpu_request = request->port;

    /*
     *	Estimate delay based on current state of time queue.
     */
    mutex_lock(&time_queue_lock);
    *delay = request_delay(request);
    mutex_unlock(&time_queue_lock);

    /*
     *	Add request port to set of ports to receive on.
     */
    port_set_add(task_self(), server_port_set, request->port);

    return (CPU_SUCCESS);
}

/*
 *	Add a processor set + processors to a request.
 */

mach_error_t
cpu_request_add(cpu_request, processor_set, processors, processors_left)
cpu_request_t	cpu_request;
processor_set_t	processor_set;
int	processors, *processors_left;
{
	request_t	request;
	mach_error_t	ret;

	if ((request = convert_port_to_request(cpu_request)) ==
	    REQUEST_NULL) {
		ret = CPU_INVALID_ARGUMENT;
		return(ret);
	}

	request_lock(request);

	if (processors <= 0) {
	    ret = CPU_INVALID_ARGUMENT;
	    goto done;
	}

	if ((request->set_count == MAX_SETS) ||
	    (request->total_processors - request->assigned_processors <
	     processors)) {
		ret = CPU_LIMIT_EXCEEDED;
		goto done;
	}

	if (request->state != REQUEST_INACTIVE) {
		ret = CPU_REQUEST_ACTIVE;
		goto done;
	}
	
	request->sets[request->set_count] = processor_set;
	request->processors[request->set_count] = processors;
	request->set_count += 1;
	request->assigned_processors += processors;
	*processors_left = request->total_processors -
		request->assigned_processors;
	
	ret = CPU_SUCCESS;
done:
	request_unlock(request);
	request_deallocate(request);
	return(ret);
}

/*
 *	cpu_request_activate_common - Common code for request activation.
 */
mach_error_t
cpu_request_activate_common(cpu_request, options, total_time, delay, task)
cpu_request_t	cpu_request;
int		options;
int		total_time;
int		*delay;
{
	request_t	request, prev_req;
	mach_error_t	ret;

	if (((options & CPU_OPTION_REPEAT) &&
		((total_time < MIN_TIME ) ||
		 (total_time > max_total_time))) ||
	    (request = convert_port_to_request(cpu_request)) ==
		REQUEST_NULL) {
			ret = CPU_INVALID_ARGUMENT;
			return(ret);
	}

	mutex_lock(&time_queue_lock);
	request_lock(request);

	if (request->state != REQUEST_INACTIVE) {
		ret = CPU_REQUEST_ACTIVE;
		goto done;
	}

	request->state = REQUEST_ACTIVATED;
	request->options |= options;
	request->task = task;
	request->total_processors = request->assigned_processors;
	request->assigned_processors = 0;
	if (options & CPU_OPTION_REPEAT) {
		request->total_time = total_time * 10;
		if (request->run_time > request->total_time)
			request->run_time = request->total_time;
	}

	/*
	 *	Insert request into time queue.
	 */
	queue_enter(&time_queue, request, request_t, active_req);
	*delay = request_delay(request);

	/*
	 * If this is the first request on the time queue, unblock the time
	 * thread to deal with it.
	 */
	if (request == queue_first(&time_queue)) {
		condition_signal(&time_queue_condition);
	}
	else {

	    /*
	     * If this is the first unassigned request and there are enough
	     * processors available, wake up the time thread to assign them.
	     */
	    prev_req = (request_t) request->active_req.prev;
	    if ((prev_req->state == REQUEST_ASSIGNED) &&
	    	(available_processors >= request->total_processors)) {
		  abort_wait();
	    }
	}

	ret = CPU_SUCCESS;

done:
	request_unlock(request);
	mutex_unlock(&time_queue_lock);
	request_deallocate(request);
	mutex_lock(&log_file_lock);
	print_time();
    	printf("Activated %d: %d processors\n", (int)request,
		request->total_processors);
	fflush(stdout);
    	mutex_unlock(&log_file_lock);
	return(ret);
}

/*
 *	Actual stubs that call cpu_request_activate_common.
 */
mach_error_t
cpu_request_activate(cpu_request, options, total_time, delay)
cpu_request_t	cpu_request;
int		options;
int		total_time;
int		*delay;
{
	return(cpu_request_activate_common(cpu_request,
		options & CPU_VALID_OPTIONS,
		total_time, delay, (task_t)0));
}

mach_error_t
cpu_request_activate_task(cpu_request, options, total_time, task, delay)
cpu_request_t	cpu_request;
int		options;
int		total_time;
task_t		task;
int		*delay;
{
	return(cpu_request_activate_common(cpu_request,
		(options & CPU_VALID_OPTIONS) | CPU_OPTION_SUSPEND,
		total_time, delay, task));
}

/*
 *	cpu_request_set_notify: indicate that request generates
 *	notifications and supply a port for them.
 */
mach_error_t
cpu_request_set_notify(cpu_request, notify_port)
cpu_request_t	cpu_request;
port_t		notify_port;
{
	request_t	request;
	mach_error_t	ret = ERR_SUCCESS;

	if ((request = convert_port_to_request(cpu_request)) ==
	    REQUEST_NULL) {
		return(CPU_INVALID_ARGUMENT);
	}
	request_lock(request);

	if (request->state == REQUEST_INACTIVE) {
		request->options |= CPU_OPTION_NOTIFY;
		request->notify_port =  notify_port;
	}
	else {
		ret = CPU_REQUEST_ACTIVE;
	}

	request_unlock(request);
	request_deallocate(request);
	return(ret);
}

	

/*
 *	cpu_request_destroy - terminate a cpu request.
 */
mach_error_t
cpu_request_destroy(cpu_request)
cpu_request_t	cpu_request;
{
	request_t	request;

	if ((request = convert_port_to_request(cpu_request)) ==
	    REQUEST_NULL) {
		return(CPU_INVALID_ARGUMENT);
	}

    	mutex_lock(&time_queue_lock);
	request_lock(request);
	switch(request->state) {
	    case REQUEST_ASSIGNED:
	       /*
		* Bring this request to the head of the time queue.
		*/
		queue_remove(&time_queue, request, request_t, active_req);
		queue_enter_first(&time_queue, request, request_t,
			active_req);
	       /*
	    	* Killing the request.  Make it appear to have timed out.
		* If an end notification was requested, this will be
		* handled as usual by the time thread.
	     	*/
	    	request->event_time = current_time();
		request->total_time = request->run_time;
	    	abort_wait();   /* abort timeout */
	    	request_unlock(request);
	    	break;
	    case REQUEST_ACTIVATED:
		queue_remove(&time_queue, request, request_t, active_req);
	       /*
	        * Fall through...
		*/
	    case REQUEST_INACTIVE:
		request_unlock(request);
		request_deallocate(request);
		break;
	    case REQUEST_DEAD:
	        request_unlock(request);
		break;
	    default:
	    	fprintf(stderr, "cpu_request_destroy: bad state.\n");
		exit(1);
	}
    	mutex_unlock(&time_queue_lock);
	
	request_deallocate(request);
	return(CPU_SUCCESS);
}

/*
 *	cpu_request_status - return info on a request.
 */

mach_error_t
cpu_request_status(cpu_request, reserved, assigned, active, options, time)
cpu_request_t	cpu_request;
int	*reserved, *assigned;
boolean_t	*active;
int	*options, *time;
{
	request_t	request;

	if ((request = convert_port_to_request(cpu_request)) ==
	    REQUEST_NULL) {
		return(CPU_INVALID_ARGUMENT);
	}

	mutex_lock(&time_queue_lock);
	request_lock(request);
	*reserved = request->total_processors;
	*assigned = request->assigned_processors;
	*time = request_delay(request);
	if (request->state == REQUEST_INACTIVE) {
	    *active = FALSE;
	    *options = CPU_OPTION_NONE;
	}
	else {
	    *active = TRUE;
	    *options = request->options;
	}
	request_unlock(request);
	mutex_unlock(&time_queue_lock);

	request_deallocate(request);

	return(CPU_SUCCESS);
}

/*
 *	Generic information about server.
 */
mach_error_t
cpu_server_info(server, time, total_time, processors, delay)
port_t	server;
int	*time, *total_time, *processors, *delay;
{
	*time = max_time;
	*total_time = max_total_time;
	*processors = max_processors;
	mutex_lock(&time_queue_lock);
	*delay = request_delay(REQUEST_NULL);
	mutex_unlock(&time_queue_lock);

	return(CPU_SUCCESS);
}
