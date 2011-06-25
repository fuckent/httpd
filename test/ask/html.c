#include "html.h"

char * 
http_html_get_request_parse(char * init_line)
{
	printf("string: %s\n", init_line);
	BOOL isGet = strncmp(init_line, "GET ", 4);
		printf("have GET\n isGet = %d\n", isGet);

	if (isGet) return NULL;
	char * tail = init_line + 4;
	//~ printf ("tail = %s\n", tail);
	int i = 0;
	while (tail[i] != ' ' && tail[i] != CR) i++;
	
	if (tail[i] == ' ')
	{
		//~ printf(" rr= %d\n", strncmp(tail+i+1,  "HTTP/1.0", 8));
		if (strncmp(tail+i+1,  "HTTP/1.1", 8) != 0)
			return NULL;

		tail[i] = 0;
	} else return NULL;
	printf("tail + 1 is %s\n", tail + 1);
	return tail+1;
}

int 
httpd_html_send_file(char * name, int fd)
{
	int r = open(name, O_RDONLY);
	char buf[100];
	char * str;
	printf("send file\n");
	if (r == -1)
	{
		r = open(NOT_FOUND_ERROR_FILE, O_RDONLY);
		str = "HTTP/1.1 404 Not Found";
	} else {
		str = "HTTP/1.1 200 OK";
	}

	struct stat st;
	fstat(r, &st);

	sprintf(buf, "%s\nConnection: close\nContent-Type: text/html\nContent-Length: %d\n\n", str, (int)st.st_size);
	

	write(fd, buf, strlen(buf));
	sendfile(fd, r, 0, st.st_size);
	
	return 0;
}
