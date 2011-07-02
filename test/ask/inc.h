#ifndef INC_H
#define INC_H


#define _GNU_SOURCE
#include <sys/types.h>
#include <sys/socket.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <netdb.h>
#include <signal.h>
#include <sys/epoll.h>
#include <mqueue.h>
#include <sys/time.h>
#include <semaphore.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <sys/stat.h>


typedef int BOOL;

#ifndef FALSE
#define FALSE					0
#endif

#ifndef TRUE
#define TRUE					!FALSE
#endif


#define BACK_LOG				10

#define MAX_EVENTS				10
#define PORT					"1234"
#define NPROCESS				1
#define NFDPROCESS				1024

#define MAX_NUMBER_FD			100
#define SIG_MPWP				50


#define ERROR(msg) \
		do { perror(msg); exit(EXIT_FAILURE); } while (0)

#ifndef MSG
#define MSG(msg) \
		do { printf(msg "\n"); } while (0)
#endif

typedef
enum
{
		SUCCESS 				=	 0,
		E_GET_ADDR_FAILED 		=	-1,
		E_BIND_FAILED			=	-2,
		E_LISTEN_FAILED			= 	-3,
		E_GET_REQUEST			=	-4,
		E_CANT_OPEN_FILE		= 	-5,
		E_TIME_OUT				= 	-6,		
} httpd_return_t;


extern mqd_t	 		mqds[NPROCESS];
extern sem_t *	 		sem;
extern int				epollfd;
extern int				fd;
extern int 				fds[MAX_NUMBER_FD];
extern pid_t 			pid_list[NPROCESS];
extern BOOL				sigpipe_flag;


#endif
