#include "machine.h"
#include "buf.h"
#include "html.h"

httpd_return_t
machine_init()
{
		mc_free_list.len = NMACHINE;
		mc_free_list.mc = &mc_array[0];
		
		int i;
		for (i = 0; i < NMACHINE; i++)
		{
			mc_array[i].next = &mc_array[(i+1) % NMACHINE];
			mc_array[i].prev = &mc_array[(i+NMACHINE - 1) % NMACHINE];
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
		list->mc = pNext;
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
	list->len++;

	return SUCCESS;
}



httpd_return_t	machine_go_to_next_state(machine_t * mc)
{
	switch (mc->state)
	{
		case MC_STATE_READ:
			machine_read_socket(mc);
			break;
		
		case MC_STATE_WRITE:
			machine_write_socket(mc);

			break;
		
		case MC_STATE_PROCESS:
			machine_process_socket(mc);
			break;
			
		case MC_STATE_OPEN:
			machine_open_socket(mc);
			break;
			
		case MC_STATE_CLOSE:
			machine_close_socket(mc);
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


httpd_return_t	
machine_read_socket(machine_t * mc)
{
	int r = recv(mc->sfd, mc->buf, BUF_SIZE, 0);
	if (r == -1)
		return E_MACHINE_READ_SOCKET;
		
	mc->size = r;
	
	mc->state = MC_STATE_PROCESS;
	
	return SUCCESS;
}

httpd_return_t	
machine_write_socket(machine_t * mc)
{
	//~ sendfile(fd, r, &size, st.st_size)
	int r = sendfile(mc->sfd, mc->ffd, &mc->curPos, MACHINE_SEND_COUNT);
	if (sigpipe_flag)
	{
		sigpipe_flag = FALSE;
		mc->state = MC_STATE_CLOSE;
		return SUCCESS;
	}


	if (r == -1)
		return E_MACHINE_SEND_FILE;
	//struct stat st;
	//fstat(mc->ffd, &st);
	if (mc->size == mc->curPos)
	{
		mc->state = MC_STATE_CLOSE;
	}
	
	return SUCCESS;
}


httpd_return_t	
machine_open_socket(machine_t * mc)
{
	//memset(mc, 0, sizeof(machine_t));
	mc->size = 0;
	mc->curPos = 0;
	mc->buf = NULL;
	mc->ffd = -1;
	mc->state = MC_STATE_READ;
	mc->buf = httpd_buf_get_buf();
	
	return SUCCESS;
}

httpd_return_t	
machine_close_socket(machine_t * mc)
{
	httpd_buf_delete_buf(mc->buf);
	epoll_ctl(epollfd, EPOLL_CTL_DEL, mc->sfd, NULL);
	close(mc->ffd);
	close(mc->sfd);
	
	mc->state = MC_STATE_DEAD;
	return SUCCESS;
}

httpd_return_t
machine_process_socket(machine_t * mc)
{
	char * fname = http_html_get_request_parse(mc->buf);

	char * str;
	int r = open(fname, O_RDONLY);
	if (r == -1)
	{
		r = open(NOT_FOUND_ERROR_FILE, O_RDONLY);
		str = "HTTP/1.1 404 Not Found";
	} else {
		str = "HTTP/1.1 200 OK";
	}

	struct stat st;
	fstat(r, &st);

	sprintf(mc->buf, "%s\nConnection: close\nContent-Type: text/html\nContent-Length: %d\n\n", str, (int)st.st_size);
	while (
	(send(mc->sfd, mc->buf, strlen(mc->buf), 0) == -1)
	&& (errno == EAGAIN  || errno == EWOULDBLOCK ||
	errno == EINTR)
	)  /*  nothing  */	;
	
	mc->size = st.st_size;
	mc->ffd = r;
	
	mc->state = MC_STATE_WRITE;

	return SUCCESS;
}
