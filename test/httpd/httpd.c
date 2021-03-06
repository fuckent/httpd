/*
 * httpd Testing.
 * httpd.c  : Nonblock socket with epoll fd manager.
 * author : Thong T. Nguyen
 *
 * Read *README* file to know about the license.
 *
 * This file based on "man getaddrinfo" example.
 *
 */

#include "httpd.h"

mqd_t           mqds[NPROCESS];
sem_t *         sem;
int             epollfd;
int             fd;
int             fds[MAX_NUMBER_FD];
pid_t           pid_list[NPROCESS];
BOOL            sigpipe_flag;
machine_t       mc_array[NMACHINE];
machine_t*      cur_mc;
machine_list_t  mc_free_list, mc_used_list;


int
main()
{
    httpd_debug_config();
    if (httpd_connect_socket_listen(PORT, &fd) != SUCCESS)
        ERROR("listen");
    
    printf("[%010ld]    listen at port 1234\n", gettime());
    
    int pid;
    int i;
    
    sem_unlink("/httpd_sem");
    sem = sem_open("/httpd_sem", O_CREAT | O_RDWR, 00700, 0);
    open_mq();

    printf("[%010ld]    forked %d processes\n", gettime(), NPROCESS);
    for (i = 0; i < NPROCESS; i++)
    {
        if ((pid = fork()) < 0)
        {
            httpd_kill_all_child();
            break;
        }

        if (pid == 0)
        {
            printf("[%010ld]    do child\n", gettime());
            httpd_do_child(i);                  /* child */
            return -1;
        }
        else pid_list[i] = pid;
    }
    
    printf("[%010ld]    do parent\n", gettime());
    httpd_do_parent();              /* parent */

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
