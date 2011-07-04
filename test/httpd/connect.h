#ifndef CONNECT_H
#define CONNECT_H

#include "inc.h"
#include "buf.h"

httpd_return_t	httpd_connect_socket_listen(char * port, int * fd);
httpd_return_t	httpd_connect_close_socket(int fd);
char *			httpd_connect_read_data(int fd, int * len);


extern int epollfd;


#endif
