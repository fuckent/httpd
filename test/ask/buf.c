#include "buf.h"

char * 
httpd_buf_get_buf()
{
	return malloc(BUF_SIZE);
}

