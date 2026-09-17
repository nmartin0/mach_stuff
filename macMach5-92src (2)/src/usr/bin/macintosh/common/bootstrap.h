typedef struct {
    mach_msg_header_t	head;
    mach_msg_type_t	MasterDevicePortType;
    mach_port_t		MasterDevicePort;
    mach_msg_type_t	HostPrivPortType;
    mach_port_t		HostPrivPort;
} bootstrap_msg_t;
