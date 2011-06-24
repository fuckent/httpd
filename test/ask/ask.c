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

static	int		fd;
//static	sem_t*	sem;

int
main()
{
	//sem_unlink("/httpd_sema");
	//sem = sem_open("/httpd_sema", O_RDWR | O_CREAT, 00700, 0);
	//if (sem == NULL) ERROR("sem_open");
	
	mqds = malloc(NPROCESS * sizeof(mqd_t));
	//MSG("malloced" "mqds");
	
	if (mqds == NULL)
		ERROR("malloc()");
		
	if (httpd_socket_listen(PORT, &fd) != SUCCESS)
		ERROR("listen");
	
	//MSG("listening at " PORT);
	
	//sleep(20);						/* sleep 20 seconds to test*/
	int pid;
	int i;

	open_mq();
	//MSG("open" "mq");

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
	
	//MSG("forked "  " processes");
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
		newfd = accept4(fd, NULL, NULL, SOCK_NONBLOCK);
		if (newfd == -1)
		{
			if (errno == EAGAIN) continue;
			//sem_post(sem);
			ERROR("accept4()");	
		}

		//sem_post(sem);
		break;
	}

	char * str = 
"HTTP/1.0 200 OK\n\
Date: Fri, 31 Dec 1999 23:59:59 GMT\n\
Content-Type: text/html\n\
Content-Length: 73\n\
\n\
<html>\n\
<body>\n\
<h1>Happy New Millennium!</h1>\n\
Hello world\n\
</body>\n\
</html>\n";
	
	write(newfd, str, strlen(str));
	//close(newfd);
	return SUCCESS;
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
	
	ev.events = EPOLLIN | EPOLLET;
	ev.data.fd = listen_sock;
	if (epoll_ctl(epollfd, EPOLL_CTL_ADD, listen_sock, &ev) == -1) {
	   perror("epoll_ctl: listen_sock");
	   exit(EXIT_FAILURE);
	}

	while (1)
	{
		nfds = epoll_wait(epollfd, events, 1 , -1);
		if (nfds == -1) ERROR("epoll_pwait");
		
		if (events[0].data.fd == listen_sock) {
			int id = httpd_get_child_ID();
			httpd_send_to_child(id);
			//sem_wait(sem);		
		}
	}
	
	return SUCCESS;
}

static httpd_return_t
httpd_setup_select()
{
	return SUCCESS;
}

static httpd_return_t
httpd_send_to_child(int childID)
{
	mq_send(mqds[childID], "sent client", sizeof("sent client"), 0);
	return SUCCESS;
}
static int current_child_ID= 0;

static int
httpd_get_child_ID()
{
	current_child_ID++;
	current_child_ID %= NPROCESS;
	
	return current_child_ID;
}

