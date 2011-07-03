#ifndef MACHINE_H
#define MACHINE_H	1

#include "inc.h"

#define NMACHINE	100


typedef enum
{
	MC_STATE_READ,
	MC_STATE_WRITE,
} mc_state_t;

typedef struct machine_t
{
	int fd;
	mc_state_t state;
	int size;
	int extra;
	void * buf;
	struct machine_t * next;
	struct machine_t * prev;
} machine_t;

typedef struct
{
	machine_t * mc;
	int len;
} machine_list_t;


httpd_return_t	machine_init();
machine_t * 	machine_get();
machine_t * 	machine_get_from_list(machine_list_t *list);
httpd_return_t	machine_remove(machine_t * mc);
httpd_return_t 	machine_add_to_list(machine_t * mc, machine_list_t * list);
httpd_return_t 	machine_remove_from_list(machine_t * mc, machine_list_t * list);
httpd_return_t	machine_go_to_next_state(machine_t * mc);

#endif
