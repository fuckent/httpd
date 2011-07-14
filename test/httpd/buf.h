#ifndef MODULES_BUF_H
#define MODULES_BUF_H           1

#define BUF_SIZE                1*1024

#include "inc.h"

char *              httpd_buf_get_buf();
httpd_return_t      httpd_buf_delete_buf(void * buf);

#endif
