/* Put this stuff in ux/server/uxkern/ether_io.c */

/*
 * Note that bsd_arp_resolve takes a devport.
 * This is a later version than the one in the netstuff.c file.
 * You'll have to hack netstuff.c to send the ether device port.
 */

/*
 * Messages from bsd_1.defs.
 */

kern_return_t
bsd_ether_device_port(proc_port, intr,
			address, address_cnt,
			device_port)
	mach_port_t	proc_port;
	boolean_t	*intr;
	struct sockaddr_in *address;
	int		address_cnt;
	mach_port_t	*device_port; 		/* out */
{
    register struct ether_softc *es;

    if (address->sa_family == AF_INET) {

	if (es_list->es_link == (struct ether_softc *)0) {
	    /* only one interface */
	    es = es_list;
	}
	else {
	    /*
	     * Find the ethernet port that has the given IP address.
	     */
	    for (es = es_list; es; es = es->es_link)
		if (es->es_ip.s_addr == ((struct sockaddr_in *)address)->sin_addr.s_addr)
		    break;
	    if (es == (struct ether_softc *)0) {
		*device_port = MACH_PORT_NULL;
		return KERN_INVALID_ADDRESS;
	    }
	}
	*device_port = es->es_port;
	return KERN_SUCCESS;
    }

    return KERN_INVALID_ADDRESS;
}

kern_return_t
bsd_arp_resolve(proc_port, intr, devport, inaddr, incnt, enaddr, encnt)
	mach_port_t	proc_port;
	boolean_t	*intr;
	mach_port_t	devport; /* network interface */
	struct sockaddr_in *inaddr;
	int		incnt;
	u_char		*enaddr;
	int		*encnt;
{
	register struct ether_softc *es;
	int usetrailers;

	/*
	 * Find the ether softc for this port.
	 */
	for (es = es_list; es; es = es->es_link)
		if (devport == es->es_port)
			break;
	if (es == (struct ether_softc *)0)
		return KERN_INVALID_RIGHT;

	/*
	 * Look in the arp table.
	 */
	if (arpresolve(&(es->es_ac), 0, &(inaddr->sin_addr), enaddr, &usetrailers)) {
		*encnt = sizeof(es->es_addr);	/* 6 */
		return KERN_SUCCESS;
	}
	else {
		/* Reply hazy, ask again later. */
		*encnt = 0;
		return KERN_INVALID_ADDRESS;
	}
}
