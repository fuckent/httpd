#include "buf.h"

char * 
httpd_buf_get_buf()
{
	return malloc(BUF_SIZE);
}

httpd_return_t 
httpd_buf_delete_buf(void * buf)
{
	free(buf);
	return SUCCESS;
}

