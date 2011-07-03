#include "do_child.h"
#include "html.h"
#include "connect.h"
#include "machine.h"

int epollfd;


static int httpd_setup_handler();
//static httpd_return_t httpd_process_client(int newfd);
static int httpd_add_fd(int fd);
static void delete_dead_jobs();
static void do_jobs();


httpd_return_t 
httpd_do_child(int id)
{
	static int first_time = 1;
	if (first_time)
	{
		epollfd = epoll_create(NFDPROCESS);
		if (epollfd == -1) ERROR("epoll_create");
		httpd_setup_handler();
		machine_init();
		first_time = 0;
	}
	
	int nfds;
	struct epoll_event events[MAX_EVENTS];
	while(1)
	{
		nfds = epoll_wait(epollfd, events, MAX_EVENTS , 0);
		if (nfds == -1) continue; //ERROR("epoll_pwait");
		
		int i;
		//printf("nfds = %d\n", nfds);
		for (i= 0; i< nfds; i++) {
			
			machine_t * mc = machine_get();
			if (mc == NULL) continue;
			
			mc->sfd = events[i].data.fd;
			mc->state = MC_STATE_OPEN;
		}
		
		do_jobs();
		delete_dead_jobs();
		
		if (mc_used_list.len == 0 && nfds == 0)
		{
			int d = sleep(200);
			printf("sleep in %d second\n", 200-d);
		}
	}
	
	return SUCCESS;
}

static void
do_jobs()
{
	int count = mc_used_list.len;
	machine_t * mc = mc_used_list.mc;
	while (count-- != 0) 
	{
		//printf("count = %d\n", count);
		machine_go_to_next_state(mc);
		mc = mc->next;
	}
}

static void
delete_dead_jobs()
{
	int count = mc_used_list.len;
	machine_t * mc = mc_used_list.mc;
	machine_t * mc_next;
	while (count-- != 0) 
	{
		mc_next = mc->next;
		if (mc->state == MC_STATE_DEAD)
			machine_remove(mc);
			
		mc = mc_next;
	}
}

static int
httpd_add_fd(int fd)
{
	struct epoll_event ev;

	ev.events = EPOLLIN|EPOLLET;
	ev.data.fd = fd;
	if (epoll_ctl(epollfd, EPOLL_CTL_ADD, fd, &ev) == -1) {
	   perror("epoll_ctl: listen_sock");
	   exit(EXIT_FAILURE);
	}
	return SUCCESS;
}



static void
sigpipe_handler(int sig, siginfo_t *si, void *unused)
{
	printf("Got a signal (%d) from %d\n", sig, si->si_pid);
	
	if (sig == SIGPIPE)
	{
		sigpipe_flag = TRUE;
		MSG("SIGPIPE");
	}
	
}

static void
sigio_handler(int sig, siginfo_t *si, void *unused)
{
	printf("Got a signal (%d) from %d\n", sig, si->si_pid);
	
	if (sig == SIGIO)
	{
		MSG("SIGIO");
	}
	
}


static void
handler(int sig, siginfo_t *si, void *unused)
{
	printf("Got a signal (%d) from %d\n", sig, si->si_pid);
	
	if (sig == SIG_MPWP)
	{
		/*int idx = get_free_fd();
		if (idx == -1) {printf("full fd\n");	return;}
		*/
		int newfd;
		newfd = accept(fd, NULL, NULL);
		fcntl(newfd, F_SETFL, O_ASYNC);
		fcntl(newfd, F_SETOWN, getpid());
		sem_post(sem);
		if (newfd == -1) return;
		MSG("add new fd");
		httpd_add_fd(newfd);	
		MSG("add new fd to this worker");
		//fds[idx] = newfd;
	}
	
}


static int
httpd_setup_handler()
{
	struct sigaction sa;
	sa.sa_flags = SA_SIGINFO;
	sigemptyset(&sa.sa_mask);
	sa.sa_sigaction = handler;
	sigaction(SIG_MPWP, &sa, NULL);

	struct sigaction sa1;
	sa1.sa_flags = SA_SIGINFO;
	sigemptyset(&sa1.sa_mask);
	sa1.sa_sigaction = sigpipe_handler;
	sigaction(SIGPIPE, &sa1, NULL);

	struct sigaction sa2;
	sa2.sa_flags = SA_SIGINFO;
	sigemptyset(&sa2.sa_mask);
	sa2.sa_sigaction = sigio_handler;
	sigaction(SIGIO, &sa2, NULL);

	return SUCCESS;
}

/*
static httpd_return_t
httpd_process_client(int newfd)
{
	char * buf = httpd_connect_read_data(newfd, NULL);
	if (buf == NULL) ERROR("httpd_read_data()");
	MSG("http_html_get_request_parse(buf)");
	char * link  = http_html_get_request_parse(buf);
	
	httpd_html_send_file(link, newfd);

	httpd_connect_close_socket(newfd);
	
	httpd_buf_delete_buf(buf);
	
	return SUCCESS;
}
*/
