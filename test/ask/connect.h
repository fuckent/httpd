#ifndef CONNECT_H
#define CONNECT_H

#include "inc.h"
#include "buf.h"

httpd_return_t	httpd_socket_listen(char * port, int * fd);
httpd_return_t	httpd_close_socket(int fd);
char *			httpd_read_data(int fd, int * len);


extern int epollfd;


#endif
