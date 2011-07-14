#include "buf.h"

char * 
httpd_buf_get_buf()
{
    printf("[%010ld]    get a buffer\n", gettime());
    return malloc(BUF_SIZE);
}

httpd_return_t 
httpd_buf_delete_buf(void * buf)
{
    printf("[%010ld]    delete a buffer\n", gettime());
    free(buf);
    return SUCCESS;
}



