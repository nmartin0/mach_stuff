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
 * $Log:	assign_test.c,v $
 * Revision 2.3  91/02/08  16:05:22  mrt
 * 	Added new Mach copyright and author notices
 * 
 * Revision 2.2  90/01/24  23:33:33  mrt
 * 	Changed include of sys/message.h to mach/message.h and
 * 	sys/error.h to mach/error.h
 * 	[90/01/22            mrt]
 * 
 * 	Created
 * 	[90/01/18            dlb]
 * 
 */
/*
 *	File: 	assign_test.c
 *	Author:	David Black, Carnegie Mellon University
 *	Date:	Jan 1990
 *
 *	assign.c - Processor assignment control.
 */

#include <mach.h>
#include <cthreads.h>
#include <mach/message.h>
#include <mach/error.h>

#include <stdio.h>

#include "request.h"

unsigned long *ctr_address, *mapfrcounter();

/*
 *	Initialize processors data structures.
 */

processor_init()
{
    int				size, i, j, k;
    int				temp;
    struct processor_basic_info	pinfo;
    struct host_basic_info	hinfo;
    processor_set_t		default_name;

    /*
     *	Get default processor set port.
     */
    processor_set_default(host_self(), &default_name);
    host_processor_set_priv(host_priv_self(), default_name, &default_set);

    /*
     *	Get processor ports, and set up max.
     */
    size = sizeof(hinfo)/sizeof(int);
    host_info(host_self(), HOST_BASIC_INFO, &hinfo, &size);
    host_processors(host_priv_self(), &processors, &processor_count);

    if (processor_count <= 1) {
	fprintf(stderr,
	    "Only %d processors; this server requires a multiprocessor!\n",
	    processor_count);
	exit(1);
    }
    max_processors = processor_count - (processor_count + 3)/4; 
    free_processors = (run_processor_t)
			malloc(sizeof(run_processor_data_t)* max_processors);
    /*
     *	Loop through and pick out processors to control.  Can't use master.
     *  Assign all to default_set for sanity.
     */
    j = 0;			/* running count of processors allocated */
    for (i = 0; i < processor_count; i++) {
	size = PROCESSOR_BASIC_INFO_COUNT;
	processor_info(processors[i], PROCESSOR_BASIC_INFO, &temp,
		&pinfo, &size);
    	if (! pinfo.is_master) {
	    if (j < max_processors) {
		free_processors[j].this_processor = processors[i];
		if (j != 0)
		    free_processors[j-1].next = &(free_processors[j]);
		free_processors[j].next = RP_NULL;
		free_processors[j].status = FREE;
		free_processors[j].slot_num = pinfo.slot_num;
		processor_assign(free_processors[j].this_processor,
			default_set, TRUE);
		j++;
	    }
	    else {
		processor_assign(processors[i], default_set, TRUE);
	    }
	}
    }
    available_processors = j;

    /*
     *	Time policy.  No more than 15 minutes per slot.
     */
    max_time = 15*60;

    /*
     *	Total time policy.  No more than 2 hours total.
     */
    max_total_time = 2*60*60;

    /*
     *	Map counter and touch it to fault it in.
     */
    ctr_address = mapfrcounter(0, TRUE);
    i = *ctr_address;
}


/*
 *	Release processors.  Assign all processors to default.
 */
release_processors(request)
request_t	request;

{
    run_processor_t this_proc, next_proc;
    unsigned long	before, after;

    before = *ctr_address;
    if (request->options & CPU_OPTION_SUSPEND) {
	task_suspend_special(request->task);
	request->resume_needed = TRUE;
    }
    for (this_proc = request->processor_ids; this_proc != RP_NULL;
	this_proc = next_proc) {
	    next_proc = this_proc->next;
	    this_proc->status = FREE;
	    this_proc->next = free_processors;
	    free_processors = this_proc;
	    available_processors++;
	    processor_assign(this_proc->this_processor, default_set, FALSE);
    }
    request->assigned_processors = 0;
    request->processor_ids = RP_NULL;
    after = *ctr_address;
    printf("Release elapsed: %d\n", after - before);
}

/*
 *	assigned_request_insert - insert an assigned request into the
 *	time_queue.  Assigned requests are ordered by time of next event.
 */
assigned_request_insert(req)
request_t	req;
{
    register request_t	scan;

    if (queue_empty(&time_queue)) {
	queue_enter(&time_queue, req, request_t, active_req);
    }
    else {
	scan = queue_first(&time_queue);
	while ((!queue_end(&time_queue,scan)) &&
	       (scan->state == REQUEST_ASSIGNED)) {
		    if (scan->event_time >= req->event_time)
			break;
			
		    scan = scan->active_req.next;
	}
	if (queue_end(&time_queue, scan)) {
	    queue_enter(&time_queue, req, request_t, active_req);
	}
	else if (scan == queue_first(&time_queue)) {
	    queue_enter_first(&time_queue, req, request_t, active_req);
	}
	else {
	    register request_t	prev;

	    /*
	     *	Insert before scan, prev elt is a real element.
	     */
	    prev = scan->active_req.prev;
	    req->active_req.next = (queue_entry_t) scan;
	    prev->active_req.next = (queue_entry_t) req;
	    req->active_req.prev = (queue_entry_t) prev;
	    scan->active_req.prev = (queue_entry_t) req;
	}
    }
}

/*
 *	Assign processors to the first n requests on time_queue so as to 
 *	use up as many of the free processors as possible.
 *	Queue must be locked.
 */
assign_processors()
{
    int set_index, count, total_count;
    run_processor_t proc_ptr;
    request_t req, next_req;
    request_t scan;
    int delay_time;
    boolean_t wait;

    /*
     *	Skip already assigned requests.
     */
    req = queue_first(&time_queue);
    while ((!queue_end(&time_queue, req)) &&
	   (req->state == REQUEST_ASSIGNED))
		req = req->active_req.next;

    /*
     *	Loop to assign as many requests in order as possible.
     */
    while ((!queue_end(&time_queue, req)) &&
      (available_processors >= req->total_processors)) {

	/*
	 *	Initialize assign loop.
	 */
	request_lock(req);
	next_req = req->active_req.next;
	queue_remove(&time_queue, req, request_t, active_req);
    	set_index = 0;
    	total_count = 0;
	count = req->processors[0];
	mutex_lock(&log_file_lock);
	print_time();
	printf("Assigning processors to %d:", req);
	wait = ((req->options & CPU_OPTION_NOTIFY) != 0);

	/*
	 * Assign processors to this request.
	 */
	while (total_count < req->total_processors) {
	    /*
	     *	Errors here are caused by clients.  Ignore them.
	     */
	    proc_ptr = free_processors;
	    free_processors = proc_ptr->next;
	    proc_ptr->status = BUSY;
	    printf(" %d",proc_ptr->slot_num);
	    processor_assign(proc_ptr->this_processor, req->sets[set_index],
			wait);
	    proc_ptr->next = req->processor_ids;
	    req->processor_ids = proc_ptr;
	    total_count++;
	    count--;
	    if (count == 0) {
	        set_index++;
	    	count = req->processors[set_index];
	    }
	}
	printf("\n");
	fflush(stdout);
	mutex_unlock(&log_file_lock);

	/*
	 *	Now place this request at the proper place in the time queue.
	 */
	req->event_time = current_time() + req->run_time;
	available_processors -= req->total_processors;
	req->state = REQUEST_ASSIGNED;
	req->assigned_processors = req->total_processors;
	assigned_request_insert(req);

	/*
	 *	Handle start options.
	 */
	if (req->options & CPU_OPTION_NOTIFY) {
	    send_notify(req, CPU_NOTIFY_START);
	}
	if ((req->options & CPU_OPTION_SUSPEND) && req->resume_needed) {
	    task_resume_special(req->task);
	    req->resume_needed = FALSE;
	}
	    
	request_unlock(req);
	req = next_req;
    }
}

/*
 *	destroy_sets() - destroy psets in request.
 */
destroy_sets(request)
request_t	request;
{
    int		i;

    for (i = 0; i < request->set_count; i++) {
	/*
	 *	Errors here are caused by users.  Ignore them.
	 */
	processor_set_destroy(request->sets[i]);
    }
}

/*
 *	Compute time until next event.  Make sure it's never negative.
 */

compute_wait_duration()
{
    int delay_time, duration;
    request_t req;

    duration = max_time+1;
    req = queue_first(&time_queue);
    while ((! queue_end(&time_queue, req)) &&
    		(req->state == REQUEST_ASSIGNED)) {
	delay_time = req->event_time - current_time();
	if (delay_time < duration) duration = delay_time;
	req = req->active_req.next;
    }
    if (duration < 0)
    	duration = 0;

    return(duration);
}

/*
 *	Body of the thread that runs around assigning processors.
 */

port_t	private_port;

void
run_time_queue()
{
    request_t cur_req, next_req;
    int duration;
    int time_now;
    
    time_thread = thread_self();
    port_allocate(task_self(), &private_port);

    mutex_lock(&time_queue_lock);

    while(1) {
	/*
	 *	Processors not assigned.  Wait on condition until
	 *	request at front of queue can be handled.
	 */
	while (queue_empty(&time_queue)) {
		condition_wait(&time_queue_condition, &time_queue_lock);
	}

	/*
	 *	Ok, assign processors.
	 */
	assign_processors();
	duration = compute_wait_duration();
	mutex_unlock(&time_queue_lock);

	/*
	 *	Now wait for time to expire or the current request to
	 *	be destroyed.
	 */
	if (duration > 0)
		wait_for(duration);

	/*
	 *	Process any events that have occurred.
	 */
	mutex_lock(&time_queue_lock);
	cur_req = queue_first(&time_queue);
	
	while ((! queue_end(&time_queue, cur_req)) &&
	  (cur_req->state == REQUEST_ASSIGNED)) {
	    /*
	     *	Check if out of events.
	     */
	    time_now = current_time();
	    if (cur_req->event_time > time_now) {
		break;
	    }

	    next_req = cur_req->active_req.next;
	    if (cur_req->options & CPU_OPTION_NOTIFY &&
	    	!(cur_req->end_notify_done)) {
		    /*
		     *	Request wants an end notify.  Send it and
		     *  give the request another second.
		     */
		    request_lock(cur_req);
		    queue_remove(&time_queue, cur_req,
			request_t, active_req);
		    cur_req->event_time = time_now + 10;
		    send_notify(cur_req, CPU_NOTIFY_END);
		    cur_req->end_notify_done = TRUE;		
		    assigned_request_insert(cur_req);
		    request_unlock(cur_req);
	    }
	    else {    
		    /*
		     * Request is finished.  Release its processors, and
		     * figure out what to do with it.
		     */
		    request_lock(cur_req);
		    queue_remove(&time_queue, cur_req,
			request_t, active_req);
		    cur_req->total_time -= cur_req->run_time;
		    release_processors(cur_req);

		    if ((cur_req->options & CPU_OPTION_REPEAT) &&
		    	(cur_req->total_time > 0)) {
			    /*
			     *	This is a repeating request with time to
			     *	go.  Restart it to continue.
			     */
			    cur_req->state = REQUEST_ACTIVATED;
			    if (cur_req->run_time > cur_req->total_time)
				cur_req->run_time = cur_req->total_time;
			    queue_enter(&time_queue, cur_req, request_t,
				active_req);
			    cur_req->end_notify_done = FALSE;
			    request_unlock(cur_req);

			    mutex_lock(&log_file_lock);
			    print_time();
			    printf("Reactivated %d: %d processors\n",
				cur_req, cur_req->total_processors);
			    fflush(stdout);
			    mutex_unlock(&log_file_lock);
		    }
		    else {
			    /*
			     * This request is finished.
			     */
			    cur_req->state = REQUEST_DEAD;
			    if (cur_req->options & CPU_OPTION_SUSPEND)
			    	task_resume_special(cur_req->task);
			    request_unlock(cur_req);
			    request_deallocate(cur_req);
		    }
	    }
	    cur_req = next_req;
	}
    }
}
	
/*
 *	wait_for - wait for time specified or until aborted.
 */
 
wait_for(time)
int	time;
{
    struct wait_for_message {
	msg_header_t	header;
    } wait_msg;
    mach_error_t	code;

    wait_msg.header.msg_local_port = private_port;
    wait_msg.header.msg_size = sizeof(wait_msg); 
    code = msg_receive(&wait_msg, RCV_TIMEOUT, time*100);
    if (code == RCV_SUCCESS || code == RCV_TIMED_OUT) {
	return;
    }

    mach_error("wait_for", code);
    exit(1);
}

/*
 *	abort_wait - abort timeout by sending message to private port.
 */

abort_wait()
{
    kern_return_t	ret;
    struct {
	msg_header_t	header;
    } abort_msg;

    abort_msg.header.msg_simple = TRUE;
    abort_msg.header.msg_size = sizeof(abort_msg);
    abort_msg.header.msg_type = MSG_TYPE_NORMAL;
    abort_msg.header.msg_remote_port = private_port;
    abort_msg.header.msg_local_port = PORT_NULL;

    ret = msg_send(&abort_msg, MSG_OPTION_NONE, 0);

    if (ret == SEND_SUCCESS) {
    	return;
    }

    mach_error("abort_wait", ret);
    exit(1);
}

/*
 *	send_notify - notify client of event.  Notification is sent with
 *		a timeout of zero.  It is the client's responsibility
 *		to prevent the notify_port from filling up.
 */
send_notify(request, type)
request_t	request;
int		type;
{
    struct {
	msg_header_t	header;
	msg_type_t	request_type;
	port_t		request_port;
    } notify_msg;

    static msg_type_t request_portType = {
		/* msg_type_name = */		MSG_TYPE_PORT,
		/* msg_type_size = */		32,
		/* msg_type_number = */		1,
		/* msg_type_inline = */		TRUE,
		/* msg_type_longform = */	FALSE,
		/* msg_type_deallocate = */	FALSE,
    };
    
    notify_msg.header.msg_simple = TRUE;
    notify_msg.header.msg_size = sizeof(notify_msg);
    notify_msg.header.msg_type = MSG_TYPE_NORMAL;
    notify_msg.header.msg_remote_port = request->notify_port;
    notify_msg.header.msg_local_port = PORT_NULL;
    notify_msg.header.msg_id = type;
    notify_msg.request_type = request_portType;
    notify_msg.request_port = request->port;

    /*
     *	All errors here are caused by clients.  Ignore them.
     */
    mutex_lock(&log_file_lock);
    print_time();
    printf("Sending notification %d for %d\n", type, request);
    fflush(stdout);
    mutex_unlock(&log_file_lock);
    msg_send(&notify_msg, SEND_TIMEOUT, 0);
}
