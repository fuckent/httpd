#ifndef MODULES_HTML_H
#define MODULES_HTML_H			1


#define E_GET_REQUEST			-1

//#define SUCCESS					0

#define E_CANT_OPEN_FILE		-2

#define CR						0xD
#define LF						0xA

#define NOT_FOUND_ERROR_FILE	"404.html"

#include <sys/types.h>
#include <sys/socket.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/stat.h>
#include <sys/sendfile.h>

typedef int BOOL;

char * 		http_html_get_request_parse(char * init_line);
int 		httpd_html_send_file(char * name, int fd);

#endif
