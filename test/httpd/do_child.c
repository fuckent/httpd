#include "do_child.h"
#include "html.h"
#include "connect.h"
#include "machine.h"
#include "time.h"


int epollfd;
BOOL sigio_flag = FALSE;

static int httpd_setup_handler();
static int httpd_add_fd(int fd);
static void delete_dead_jobs();
static void do_jobs();


httpd_return_t 
httpd_do_child(int id)
{
    static int first_time = 1;
    if (first_time)
    {
        printf("[%010ld]    i'm a new child (pid : %d)\n",gettime(), getpid());
        printf("[%010ld]    init handler and machines\n", gettime());
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
        if (sigio_flag)
        {
            sigio_flag = 0;
            {
                int newfd;
                newfd = accept(fd, NULL, NULL);
                sem_post(sem);
                // if (newfd == -1) return;
                httpd_add_fd(newfd);    
            }
        }

        nfds = epoll_wait(epollfd, events, MAX_EVENTS , 0);

#ifdef  LOG_FILE
        fflush(stdout);
#endif

        if (nfds == -1) continue; //ERROR("epoll_pwait");
        
        int i;
        //printf("nfds = %d\n", nfds);
        for (i= 0; i< nfds; i++) {
            printf("[%010ld]    get new machine\n", gettime());
            machine_t * mc = machine_get();
            if (mc == NULL) continue;
            
            mc->sfd = events[i].data.fd;
            mc->state = MC_STATE_OPEN;
        }
        
        //printf("[%010ld]  do jobs\n", gettime());
        do_jobs();
        //printf("[%010ld]  delete dead jobs\n", gettime());
        delete_dead_jobs();
        
        /*if (mc_used_list.len == 0 && nfds == 0)
            pause(); */
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
        //printf("[%010ld]  do a job\n", gettime());
        cur_mc = mc;
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
        {
            //printf("[%010ld]  delete a job\n", gettime());
            machine_remove(mc);
        }
            
        mc = mc_next;
    }
}

static int
httpd_add_fd(int fd)
{
    struct epoll_event ev;

    ev.events = EPOLLIN|EPOLLET;
    ev.data.fd = fd;
    //printf("[%010ld]    added a fd (%d) to watch\n",gettime(), fd);
    if (epoll_ctl(epollfd, EPOLL_CTL_ADD, fd, &ev) == -1) {
       perror("epoll_ctl: listen_sock");
       exit(EXIT_FAILURE);
    }
    return SUCCESS;
}



static void
sigpipe_handler(int sig, siginfo_t *si, void *unused)
{
    //printf("[%010ld]    got a SIGPIPE signal from %d\n",gettime(), si->si_pid);
    
    sigpipe_flag = TRUE;
}

/*
static void
sigio_handler(int sig, siginfo_t *si, void *unused)
{
    sigio_flag = TRUE;
    //printf("[%010ld]    got a SIGIO signal from %d\n",gettime(), si->si_pid);
}
*/

static void
handler(int sig, siginfo_t *si, void *unused)
{
    //printf("[%010ld]    got a signal NEW JOB from %d\n",gettime(), si->si_pid);
    sigio_flag = 1;
        //~ sigio_flag = TRUE;
}


static int
httpd_setup_handler()
{
    printf("[%010ld]    set up SIG_MPWP signal\n", gettime());
    struct sigaction sa;
    sa.sa_flags = SA_SIGINFO;
    sigemptyset(&sa.sa_mask);
    sa.sa_sigaction = handler;
    sigaction(SIG_MPWP, &sa, NULL);

    printf("[%010ld]    set up SIGPIPE signal\n", gettime());
    struct sigaction sa1;
    sa1.sa_flags = SA_SIGINFO;
    sigemptyset(&sa.sa_mask);
    sa1.sa_sigaction = sigpipe_handler;
    sigaction(SIGPIPE, &sa1, NULL);

/*  printf("[%010ld]    set up SIGIO signal\n", gettime());
    struct sigaction sa2;
    sa2.sa_flags = SA_SIGINFO;
    sigemptyset(&sa.sa_mask);
    sa2.sa_sigaction = sigio_handler;
    sigaction(SIGIO, &sa2, NULL);
*/
    return SUCCESS;
}

