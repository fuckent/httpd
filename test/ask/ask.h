#ifndef ASK_H
#define ASK_H

#define _GNU_SOURCE

#include <sys/types.h>
#include <sys/socket.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <netdb.h>
#include <signal.h>
#include <sys/epoll.h>
#include <mqueue.h>
#include <sys/time.h>
#include <semaphore.h>
#include <fcntl.h>
#include <sys/stat.h>

#define BACK_LOG				10
#define MAX_EVENTS				10
#define PORT					"1234"
#define NPROCESS				4

typedef
enum
{
		SUCCESS = 0,
		E_GET_ADDR_FAILED,
		E_BIND_FAILED,
		E_LISTEN_FAILED,
} httpd_return_t;



#define ERROR(msg) \
		do { perror(msg); exit(EXIT_FAILURE); } while (0)

#define MSG(msg) \
		do { printf(msg "\n"); } while (0)

static httpd_return_t	httpd_socket_listen(char * port, int * fd);
static httpd_return_t	httpd_kill_all_child();
static httpd_return_t	httpd_do_child(int id);
static httpd_return_t	httpd_add_child(int);
static httpd_return_t	httpd_do_parent();
static httpd_return_t	httpd_setup_select();
static httpd_return_t	httpd_send_to_child(int childID);
static int				httpd_get_child_ID();
static httpd_return_t 	open_mq();
static httpd_return_t	httpd_process_client();

#endif
