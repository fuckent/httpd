/*
 * httpd Testing.
 * ask.c  : Nonblock socket with epoll fd manager.
 * author : Thong T. Nguyen
 *
 * Read *README* file to know about the license.
 *
 * This file based on "man getaddrinfo" example.
 *
 */

#include "ask.h"

static mqd_t * mqds;
static sem_t * sem;
static	int		fd;

int
main()
{
	mqds = malloc(NPROCESS * sizeof(mqd_t));
	
	if (mqds == NULL)
		ERROR("malloc()");
		
	if (httpd_socket_listen(PORT, &fd) != SUCCESS)
		ERROR("listen");
	
	int pid;
	int i;
	
	sem_unlink("/httpd_sem");
	sem = sem_open("/httpd_sem", O_CREAT | O_RDWR, 00700, 0);
	open_mq();

	signal(SIGPIPE, SIG_IGN);
	
	for (i = 0; i < NPROCESS; i++)
	{
		if ((pid = fork()) < 0)
		{
			httpd_kill_all_child();
			break;
		}

		if (pid == 0)
			httpd_do_child(i);					/* child */
		else
			httpd_add_child(pid);
	}
	
	httpd_do_parent();				/* parent */

	return SUCCESS;
}


static httpd_return_t
open_mq()
{
	char str[100];
	int i;
	for (i = 0; i < NPROCESS; i++)
	{
		sprintf(str, "/httpd_mq_%d", i);
		mq_unlink(str);
		if ((mqds[i] = mq_open(str, O_RDWR | O_CREAT, 00700, NULL)) == (mqd_t)-1)
			ERROR("mq_open()");
	}
	
	return SUCCESS;
}


static httpd_return_t
httpd_socket_listen(char * port, int * fd)
{
	struct addrinfo hints;
	struct addrinfo *result, *rp;
	int sfd, s;
	struct sockaddr_storage;
	
	memset(&hints, 0, sizeof(struct addrinfo));
	hints.ai_family = AF_UNSPEC;    /* Allow IPv4 or IPv6 */
	hints.ai_socktype = SOCK_STREAM; /* Stream and nonblock socket */
	hints.ai_flags = AI_PASSIVE;    /* For wildcard IP address */
	
	s = getaddrinfo(NULL, port, &hints, &result);
	if (s != 0) return E_GET_ADDR_FAILED;
	
	for (rp = result; rp != NULL; rp = rp->ai_next) {
		sfd = socket(rp->ai_family, rp->ai_socktype ,
		rp->ai_protocol);
		if (sfd == -1)
			continue;
		
		int yes = 1;
		if (setsockopt(sfd, SOL_SOCKET, SO_REUSEADDR,
		  (char *) &yes, sizeof(yes)) == -1) {
			perror("setsockopt");
			close(sfd);
			return -1;
		}
		
		if (bind(sfd, rp->ai_addr, rp->ai_addrlen) == 0)
			break;		    /* Success */
		
		close(sfd);
	}
	
	if (rp == NULL) return E_BIND_FAILED;
	
	freeaddrinfo(result);	    /* No longer needed */
	
	if (listen(sfd, BACK_LOG) == -1) return E_LISTEN_FAILED;
	
	*fd = sfd;
	return SUCCESS;
}


static httpd_return_t 
httpd_kill_all_child()
{
	return SUCCESS;
}


static httpd_return_t 
httpd_do_child(int id)
{
	struct mq_attr mqa;
	mq_getattr(mqds[id], &mqa);
	char * buf = malloc(mqa.mq_msgsize);
	
	while (1)
	{
		int r = mq_receive(mqds[id], buf, mqa.mq_msgsize, NULL);
		printf("#%d received %s\n", id, buf);

		if (r == -1)
			ERROR("mq_receive()");
			
		if (strcmp(buf, "sent client") == 0)
		{
			httpd_process_client();
		}
	}
	
	return SUCCESS;
}


static httpd_return_t
httpd_process_client()
{
	int newfd;

	while (1)
	{
		newfd = accept(fd, NULL, NULL);
		if (newfd == -1)
		{
			if (errno == EAGAIN) continue;
			sem_post(sem);
			ERROR("accept()");	
		}

		break;
	}
	sem_post(sem);
	char * buf = httpd_read_data(newfd, NULL);
	if (buf == NULL) ERROR("httpd_read_data()");
	MSG("http_html_get_request_parse(buf)");
	char * link  = http_html_get_request_parse(buf);
	
	httpd_html_send_file(link, newfd);

	httpd_close_socket(newfd);
	
	return SUCCESS;
}


static httpd_return_t
httpd_close_socket(int fd)
{
	printf("shut down fd = %d\n", fd);
	close(fd);
	
	return SUCCESS;
}


static char *
httpd_read_data(int fd, int* len)
{
	char * buf = httpd_buf_get_buf(); /* BUF_SIZE */
	if (buf == NULL) return NULL;
	int extra = BUF_SIZE;
	char * temp = buf;
	while (extra != 0) 
	{
		
		int r = recv(fd, temp, extra, 0);
		printf("r = %d\n", r);
		if (r == -1 && errno != EAGAIN && errno != EWOULDBLOCK)
			ERROR("recv");
		
		if (r == 0) return NULL;
		
		
		extra -= r;
		temp += r;
		break;
	}
	if (len != NULL)
		*len = BUF_SIZE - extra;
	
	return buf;
}


static httpd_return_t 
httpd_add_child(int pid)
{
	return SUCCESS;
}


static httpd_return_t 
httpd_do_parent()
{
	fd_set set;
	FD_ZERO(&set);
	FD_SET(fd, &set);	

	struct epoll_event ev, events[MAX_EVENTS];
	int listen_sock = fd, nfds, epollfd;

	epollfd = epoll_create(1);
	if (epollfd == -1) ERROR("epoll_create");
	
	ev.events = EPOLLIN;
	ev.data.fd = listen_sock;
	if (epoll_ctl(epollfd, EPOLL_CTL_ADD, listen_sock, &ev) == -1) {
	   perror("epoll_ctl: listen_sock");
	   exit(EXIT_FAILURE);
	}

	while (1)
	{
		nfds = epoll_wait(epollfd, events, MAX_EVENTS , -1);
		if (nfds == -1) ERROR("epoll_pwait");
		
		int i;
		printf("nfds = %d\n", nfds);
		for (i= 0; i< nfds; i++) {
			if (events[i].data.fd == listen_sock) {
				int id = httpd_get_child_ID();
				httpd_send_to_child(id);
			}
		}
	}
	
	return SUCCESS;
}

/*
static httpd_return_t
httpd_setup_select()
{
	return SUCCESS;
}
*/

static httpd_return_t
httpd_send_to_child(int childID)
{
	mq_send(mqds[childID], "sent client", sizeof("sent client"), 0);
	sem_wait(sem);
	return SUCCESS;
}


static int
httpd_get_child_ID()
{
	static int current_child_ID = 0;
	current_child_ID++;
	current_child_ID %= NPROCESS;
	
	return current_child_ID;
}
