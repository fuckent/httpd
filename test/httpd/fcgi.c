#include "fcgi.h"
#include "machine.h"

/*
 * return:  -   SUCCESS
 *          -   E_SOCKET_CONNECT
 */
 
httpd_return_t fcgi_create_record(char* buf, int type, int id);
int fcgi_create_pair_name_value(char* buf, char * name, char * value);

httpd_return_t
fcgi_socket_connect(char * addr, char * port, int * fd)
{
    struct addrinfo hints;
    struct addrinfo *result, *rp;
    int sfd, s;
    
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_UNSPEC;                /* Allow IPv4 or IPv6 */
    hints.ai_socktype = SOCK_DGRAM;             /* Datagram socket */
    hints.ai_flags = 0;
    hints.ai_protocol = 0;                      /* Any protocol */

    s = getaddrinfo(addr, port, &hints, &result);
    if (s != 0) {
        return E_SOCKET_CONNECT;
        ERROR("getaddrinfo()");
    }

    for (rp = result; rp != NULL; rp = rp->ai_next) {
        sfd = socket(rp->ai_family, rp->ai_socktype,
                    rp->ai_protocol);
        if (sfd == -1)
            continue;

        if (connect(sfd, rp->ai_addr, rp->ai_addrlen) != -1)
            break;                          /* Success */

        close(sfd);
    }

    if (rp == NULL) {                       /* No address succeeded */
        return E_SOCKET_CONNECT;
        ERROR("Could not connect\n");
    }

    freeaddrinfo(result);                   /* No longer needed */
    *fd = sfd;
    return SUCCESS;
}

httpd_return_t
fcgi_send_request(int fd, int requestId)
{
    char * buf = cur_mc->buf;
    
    fcgi_create_record(buf, FCGI_BEGIN_REQUEST, requestId);
    write(fd, buf, 16);

    fcgi_create_record(buf, FCGI_PARAMS, requestId);
/*    
    FCGI_Record_t * re = (FCGI_Record_t*) buf;
    char *content = (char*)(&re->ContentData);
    int r = 0;

    r += fcgi_create_pair_name_value(content + r, "QUERY_STRING", fcgi_get_query_string());
    r += fcgi_create_pair_name_value(content + r, "REQUEST_METHOD", fcgi_get_request_method());
    r += fcgi_create_pair_name_value(content + r, "CONTENT_TYPE", fcgi_get_content_type());
    r += fcgi_create_pair_name_value(content + r, "CONTENT_LENGTH", fcgi_get_content_length());
    r += fcgi_create_pair_name_value(content + r, "SCRIPT_NAME", fcgi_get_script_name());
    r += fcgi_create_pair_name_value(content + r, "REQUEST_URI", fcgi_get_request_uri());
    r += fcgi_create_pair_name_value(content + r, "DOCUMENT_URI", fcgi_get_document_uri());
    r += fcgi_create_pair_name_value(content + r, "DOCUMENT_ROOT", fcgi_get_document_root());
    r += fcgi_create_pair_name_value(content + r, "SERVER_PROTOCOL", fcgi_get_server_protocol());
    r += fcgi_create_pair_name_value(content + r, "GATEWAY_INTERFACE", "CGI/1.1");
    r += fcgi_create_pair_name_value(content + r, "REMOTE_ADDR", "CGI/1.1");
    r += fcgi_create_pair_name_value(content + r, "SERVER_NAME", "CGI/1.1");
    r += fcgi_create_pair_name_value(content + r, "SERVER_PORT", "CGI/1.1");
*/
    return SUCCESS;
}


httpd_return_t
fcgi_get_respond()
{
    return SUCCESS;
}

httpd_return_t
fcgi_close_session()
{
    return SUCCESS;
}

httpd_return_t
fcgi_create_record(char* buf, int type, int id)
{
    FCGI_Record_t * pRecord = (FCGI_Record_t*)buf;
    
    pRecord->version    = FCGI_VERSION_1;
    pRecord->type       = type;
    pRecord->requestId  = htobe16(id);
    
    switch (type)
    {
        case FCGI_BEGIN_REQUEST:
        {
            FCGI_BeginRequestBody_t *content = (FCGI_BeginRequestBody_t*)(&pRecord->ContentData);
            content->role = htobe16(FCGI_RESPONDER);
            content->flags = 0;
            pRecord->contentlength = 8;
            pRecord->paddingLength = 0;
            break;
        }
        case FCGI_PARAMS:
        {
            break;
        }
    }
    
    return SUCCESS;
}


int
fcgi_create_pair_name_value(char* buf, char * name, char * value)
{
    int l1  = strlen(name) - 1;
    int l2  = strlen(value) - 1;
    int c   = ((l1 >127) << 1) | (l2 >127);
    switch (c)
    {
        case 0:
        {
            FCGI_NameValuePair11_t* t1 = (FCGI_NameValuePair11_t*)buf;
            t1->nameLength  = l1;
            t1->valueLength = l2;
            strncpy(buf + 2, name, l1);
            strncpy(buf + 2 + l1, value, l2);
            return l1+l2+2;
            break;
        }
        case 1:
        {
            FCGI_NameValuePair14_t* t1 = (FCGI_NameValuePair14_t*)buf;
            t1->nameLength  = l1;
            t1->valueLength = htobe32(l2 | (1<<31));
            strncpy(buf + 5, name, l1);
            strncpy(buf + 5 + l1, value, l2);
            return l1+l2+5;
            break;
        }
        case 2:
        {
            FCGI_NameValuePair41_t* t1 = (FCGI_NameValuePair41_t*)buf;
            t1->nameLength  = htobe32(l1 | (1<<31));
            t1->valueLength = l2;
            strncpy(buf + 5, name, l1);
            strncpy(buf + 5 + l1, value, l2);
            return l1+l2+5;
            break;
        }
        case 3:
        {
            FCGI_NameValuePair44_t* t1 = (FCGI_NameValuePair44_t*)buf;
            t1->nameLength  = htobe32(l1 | (1<<31));
            t1->valueLength = htobe32(l2 | (1<<31));
            strncpy(buf + 8, name, l1);
            strncpy(buf + 8 + l1, value, l2);
            return l1+l2+8;
            break;
        }
    
    }
    return 0;

}
