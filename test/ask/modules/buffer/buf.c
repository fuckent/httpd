#include "ask.h"
#include "html.h"

static int 
http_get_request_parse(char * init_line)
{
	BOOL isGet = strncmp(init_line, "GET ", 4);
	char * tail = init_line + 5;
	
	int i = 0;
	char url[1000];
	while (tail[i] != ' ' && tail[i] != CR && i < 1000)
	{
		url[i] = tail[i];
		i++;
	}
	url[i] = 0;
	
	if (tail[i] == ' ')
	{
		if (strncmp(tail+1,  "HTTP/1.0", 8)!= 0)
			return E_GET_REQUEST;
	} else return E_GET_REQUEST;
	
	return SUCCESS;
}
