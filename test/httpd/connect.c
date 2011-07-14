#include "connect.h"


int
httpd_connect_close_socket(int fd)
{
    printf("shut down fd = %d\n", fd);
    epoll_ctl(epollfd, EPOLL_CTL_DEL, fd, NULL);
    close(fd);

    return 0;
}


char *
httpd_connect_read_data(int fd, int* len)
{
    char * buf = httpd_buf_get_buf(); /* BUF_SIZE */
    if (buf == NULL) 
        return NULL;
        
    int extra = BUF_SIZE;
    char * temp = buf;
    while (extra != 0) 
    {
        int r = recv(fd, temp, extra, 0);
        printf("r = %d\n", r);
        
        if (r == -1 && errno != EAGAIN && errno != EWOULDBLOCK)
            ERROR("recv");
        
        if (r == 0) return NULL;
        
        extra -= r;
        temp += r;
        break;
    }
    
    if (len != NULL)
        *len = BUF_SIZE - extra;
    
    return buf;
}

httpd_return_t
httpd_connect_socket_listen(char * port, int * fd)
{
    struct addrinfo hints;
    struct addrinfo *result, *rp;
    int sfd, s;
    struct sockaddr_storage;
    
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_UNSPEC;    /* Allow IPv4 or IPv6 */
    hints.ai_socktype = SOCK_STREAM; /* Stream and nonblock socket */
    hints.ai_flags = AI_PASSIVE;    /* For wildcard IP address */
    
    s = getaddrinfo(NULL, port, &hints, &result);
    if (s != 0) return E_GET_ADDR_FAILED;
    
    for (rp = result; rp != NULL; rp = rp->ai_next) {
        sfd = socket(rp->ai_family, rp->ai_socktype ,
        rp->ai_protocol);
        if (sfd == -1)
            continue;
        
        int yes = 1;
        if (setsockopt(sfd, SOL_SOCKET, SO_REUSEADDR,
          (char *) &yes, sizeof(yes)) == -1) {
            perror("setsockopt");
            close(sfd);
            return -1;
        }
        
        if (bind(sfd, rp->ai_addr, rp->ai_addrlen) == 0)
            break;          /* Success */
        
        close(sfd);
    }
    
    if (rp == NULL) return E_BIND_FAILED;
    
    freeaddrinfo(result);       /* No longer needed */
    
    if (listen(sfd, BACK_LOG) == -1) return E_LISTEN_FAILED;
    
    *fd = sfd;
    return SUCCESS;
}
