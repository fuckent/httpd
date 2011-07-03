#include "machine.h"

static machine_t		mc_array[NMACHINE];
static machine_list_t	mc_free_list, mc_used_list;


httpd_return_t
machine_init()
{
		mc_free_list.len = NMACHINE;
		mc_free_list.mc = &mc_array[0];
		
		int i;
		for (i = 0; i < NMACHINE; i++)
		{
			mc_array[i].next = &mc_array[(i+1) % NMACHINE];
			mc_array[i].prev = &mc_array[(i-1) % NMACHINE];
		}
		
		mc_used_list.len	= 0;
		mc_used_list.mc		= NULL;
		
		return SUCCESS;
}

machine_t * 	
machine_get()
{

	machine_t * pmc = machine_get_from_list(&mc_free_list);
	
	machine_add_to_list(pmc, &mc_used_list);
	
	return pmc;
}

machine_t * 	
machine_get_from_list(machine_list_t *list)
{
	if (list->len == 0)
		return NULL;
	
	machine_t * pmc = list->mc;
	machine_t * pPrev = pmc->prev;
	machine_t * pNext = pmc->next;
	if (list->len == 1)
	{
		list->mc = NULL;
	} else
	{
		pPrev->next = pNext;
		pNext->prev = pPrev;
	}
	list->len --;
	
	return pmc;
}

httpd_return_t 	machine_add_to_list(machine_t * mc, machine_list_t * list)
{
	if (mc == NULL) return SUCCESS;
	
	if (list->len == 0)
	{
		list->mc = mc;
	} else
	{
		machine_t * pPrev = list->mc->prev;
		pPrev->next = mc;
		mc->prev = pPrev;
		mc->next = list->mc;
		list->mc->prev = mc;
	}
	list->len--;

	return SUCCESS;
}



httpd_return_t	machine_go_to_next_state(machine_t * mc)
{
	switch (mc->state)
	{
		case MC_STATE_READ:
			break;
		
		case MC_STATE_WRITE:
			break;
		
		default:
			return E_MACHINE_INVAILD_STATE;
	}
	
	return SUCCESS;
} 


httpd_return_t
machine_remove(machine_t * mc)
{
	machine_remove_from_list(mc, &mc_used_list);
	machine_add_to_list(mc, &mc_free_list);
	
	return SUCCESS;
}

httpd_return_t 	
machine_remove_from_list(machine_t * mc, machine_list_t * list)
{
	if (mc == NULL)
		return SUCCESS;
	
	if (list->len == 0)
		return E_MACHINE_EMPTY;
	
	if (list->len == 1)
	{
		list->mc = NULL;
	} else 
	{
		machine_t * pPrev = mc->prev;
		machine_t * pNext = mc->next;
		pPrev->next = pNext;
		pNext->prev = pPrev;
		
		if (list->mc == mc)
			list->mc = pNext;
	}
	
	list->len --;
	
	return SUCCESS;
}
