#ifndef MODULES_HTML_H
#define MODULES_HTML_H          1


#define CR                      0xD
#define LF                      0xA

#define NOT_FOUND_ERROR_FILE    "404.html"

#include "inc.h"

char *      http_html_get_request_parse(char * init_line);
int         httpd_html_send_file(char * name, int fd);

#endif
