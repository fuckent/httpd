#include "buf.h"

char * 
httpd_buf_get_buf()
{
	MSG("[   buf.c]	get a buffer");
	return malloc(BUF_SIZE);
}

httpd_return_t 
httpd_buf_delete_buf(void * buf)
{
	MSG("[   buf.c]	delete a buffer");
	free(buf);
	return SUCCESS;
}



