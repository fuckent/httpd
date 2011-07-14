#include "do_parent.h"
#include "time.h"

static httpd_return_t   httpd_send_to_child(int childID);
static int              httpd_get_child_ID();
static int              httpd_send_signal_to_child(int id);
static httpd_return_t   httpd_create_new_child(int id);
static int              httpd_setup_handler();
static void             child_handler(int sig, siginfo_t *si, void *unused);
static BOOL             httpd_is_alive(int id);


httpd_return_t 
httpd_do_parent()
{
    httpd_setup_handler();
    fd_set set;
    FD_ZERO(&set);
    FD_SET(fd, &set);   

    struct epoll_event ev, events[MAX_EVENTS];
    int listen_sock = fd, nfds, epollfd;

    epollfd = epoll_create(MAX_EVENTS);
    if (epollfd == -1) ERROR("epoll_create");
    
    ev.events = EPOLLIN;
    ev.data.fd = listen_sock;
    if (epoll_ctl(epollfd, EPOLL_CTL_ADD, listen_sock, &ev) == -1) {
       perror("epoll_ctl: listen_sock");
       exit(EXIT_FAILURE);
    }
    int count = 0;
    printf("[%010ld]    setup epoll catch listen sock event\n", gettime());
    while (1)
    {
        while ((nfds = epoll_wait(epollfd, events, MAX_EVENTS , -1)) == -1 && errno == EINTR) ;
#ifdef  LOG_FILE
        fflush(stdout);
#endif
        
        if (nfds == -1) ERROR("epoll_pwait");
		count += nfds;
	    printf("[%010ld]    count : %d\n", gettime(), count);
        int i;
        for (i= 0; i< nfds; i++) {
            if (events[i].data.fd == listen_sock) {
                int id = httpd_get_child_ID();
                if (!httpd_is_alive(id))
                {
                    httpd_create_new_child(id);
                    sleep(1);
                }
                if (httpd_send_to_child(id) == E_TIME_OUT)
                {
                    kill(pid_list[id], SIGKILL);
                    printf("[%010ld]    killed %d\n (TIME OUT)", gettime(), pid_list[id]);

                    httpd_create_new_child(id);
                }
            }
        }
    }
    
    return SUCCESS;
}


static httpd_return_t
httpd_send_to_child(int childID)
{
    
    printf("[%010ld]    send client to %d\n", gettime(), pid_list[childID]);
    //mq_send(mqds[childID], "sent client", sizeof("sent client"), 0);
    if (httpd_send_signal_to_child(childID) < 0) ;
    //  ERROR("kill()");
    struct timespec ts;
    if (clock_gettime(CLOCK_REALTIME, &ts) == -1)
        ERROR("clock_gettime()");
    
    
    ts.tv_sec += 2;
    
    int s=0;
    sem_wait(sem);
    //while ((s = sem_timedwait(sem, NULL)) == -1 && errno == EINTR)
    //  continue;       /* Restart if interrupted by handler */
    
    if (s == -1)
    {
        if (errno == ETIMEDOUT)
            return E_TIME_OUT;
        ERROR("sem_timedwait()");
    }
    
    return SUCCESS;
}



/*
 * Return:
 *          0       SUCCESS
 *          1       FAILED
 */
int
httpd_send_signal_to_child(int id)
{
/** TODO:
 * 1. Check id (0 <id < NPROCESS) 
 * 2. send a signal to child
 **/
    return kill(pid_list[id], SIG_MPWP);
}


static int
httpd_get_child_ID()
{
    static int current_child_ID = 0;
    current_child_ID++;
    current_child_ID %= NPROCESS;
    
    return current_child_ID;
}


httpd_return_t 
httpd_kill_all_child()
{
    return SUCCESS;
}

static httpd_return_t 
httpd_create_new_child(int id)
{
    int pid;
    if ((pid = fork()) < 0)
    {
        return -1;
    }

    if (pid == 0)
    {
        httpd_do_child(id);                 /* child */
        exit(-1);
    }
    else pid_list[id] = pid;

    return SUCCESS;
}


static void
child_handler(int sig, siginfo_t *si, void *unused)
{
    // int pid = wait(NULL);
    printf("[%010ld]    a process (pid:%d) dead\n", gettime(), si->si_pid);
/*  int i;
    for (i = 0; i < NPROCESS; i++)
        if (pid_list[i] == si-> si_pid)
        {
            httpd_create_new_child(i);
        }
*/
}

static int
httpd_setup_handler()
{
    printf("[%010ld]    set up handler from dead child\n", gettime());
    struct sigaction sa;
    sa.sa_flags = SA_SIGINFO | SA_NOCLDWAIT |SA_NODEFER;
    sigemptyset(&sa.sa_mask);
    sa.sa_sigaction = child_handler;
    return sigaction(SIGCHLD, &sa, NULL);
}

static BOOL httpd_is_alive(int id)
{
    kill(pid_list[id], 0);
    return !(errno == ESRCH);
}


