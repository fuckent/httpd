#include "html.h"
#include "time.h"

char *
http_html_get_request_parse(char * init_line)
{
    //printf("string: %s\n", init_line);
    BOOL isGet = strncmp(init_line, "GET ", 4);
    //  printf("have GET\n isGet = %d\n", isGet);

    if (isGet) return NULL;
    char * tail = init_line + 4;
    //~ printf ("tail = %s\n", tail);
    int i = 0;
    while (tail[i] != ' ' && tail[i] != CR) i++;

    if (tail[i] == ' ')
    {
        //~ printf(" rr= %d\n", strncmp(tail+i+1,  "HTTP/1.0", 8));
        if (strncmp(tail+i+1,  "HTTP/1.", 7) != 0)
            return NULL;

        tail[i] = 0;
    } else return NULL;
    printf("[%010ld]    request file %s\n", gettime(), tail + 1);
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

    off_t size = 0;
    send(fd, buf, strlen(buf), 0);
    while (sendfile(fd, r, &size, st.st_size) != 0)
    {
        if (sigpipe_flag)
        {
            sigpipe_flag = FALSE;
            break;
        }
        MSG("sent again");
    }
    
    MSG("hi, i'm httpd");

    return 0;
}
