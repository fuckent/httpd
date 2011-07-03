#ifndef MACHINE_H
#define MACHINE_H	1

#include "inc.h"

#define NMACHINE				100
#define MACHINE_SEND_COUNT		50*1024 /* 50KB */

typedef enum
{
	MC_STATE_READ,
	MC_STATE_WRITE,
	MC_STATE_PROCESS,
	MC_STATE_CLOSE,
	MC_STATE_OPEN,
	MC_STATE_DEAD,
} mc_state_t;

typedef struct machine_t
{
	int sfd;
	int ffd;
	mc_state_t state;
	int size;
	off_t curPos;
	void * buf;
	struct machine_t * next;
	struct machine_t * prev;
} machine_t;

typedef struct
{
	machine_t * mc;
	int len;
} machine_list_t;


extern machine_t		mc_array[NMACHINE];
extern machine_list_t	mc_free_list, mc_used_list;


httpd_return_t	machine_init();
machine_t * 	machine_get();
machine_t * 	machine_get_from_list(machine_list_t *list);
httpd_return_t	machine_remove(machine_t * mc);
httpd_return_t	machine_read_socket(machine_t * mc);
httpd_return_t	machine_open_socket(machine_t * mc);
httpd_return_t	machine_process_socket(machine_t * mc);
httpd_return_t	machine_close_socket(machine_t * mc);
httpd_return_t	machine_write_socket(machine_t * mc);
httpd_return_t 	machine_add_to_list(machine_t * mc, machine_list_t * list);
httpd_return_t 	machine_remove_from_list(machine_t * mc, machine_list_t * list);
httpd_return_t	machine_go_to_next_state(machine_t * mc);

#endif
