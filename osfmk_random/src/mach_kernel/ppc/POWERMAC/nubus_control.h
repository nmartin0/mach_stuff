#ifndef __nubus_control_h__

struct nubus_controller {
	char *name;
	int bottom_slot;
	int top_slot;
	void		(*set_slot_disable)(unsigned short, boolean_t);
	boolean_t	(*get_slot_disable)(unsigned short);
	void		(*set_nubus_speed)(boolean_t);
	boolean_t	(*get_nubus_speed)(void);
	boolean_t	(*set_burst_enable)(unsigned short, boolean_t);
	boolean_t	(*get_burst_enable)(unsigned short);
	void		(*reset)(void);
};

typedef struct nubus_controller *nubus_controller_t;

void nubus_register_controller(nubus_controller_t controller);

#define __nubus_control_h__
#endif // __nubus_control_h__
