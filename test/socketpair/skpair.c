#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <time.h> 
#include <sys/types.h>
#include <sys/socket.h>

#include "skpair.h"

int sockets[2];


void
get_data(int nCount) {
	char * buf = malloc(MEM_SIZE); /* 1MB */
	if (buf == NULL) exit(2);
	int count = 0;
	
	while (count < nCount) {
		int l = 0;
		while ( l != MEM_SIZE){
			l += recv(sockets[0], buf + l, MEM_SIZE, 0);

		}
		//~ printf("l= %d\n", l);		

		if (buf[MEM_SIZE-10*count] != count%128) {
			printf("Received error  buf[%d]=%c\n", count, buf[count]);
			exit(1);
		}
		send(sockets[0], "ack", 4, 0);
		// printf("Received [%d]\n", count);
		count ++;
	}
}

void
send_data(int nCount) {
	char * buf = malloc(MEM_SIZE); /* 1MB */
	if (buf == NULL)
		{exit(1);}
	int count = 0;
	char buf_[10];
	while (count < nCount) {
		memset(buf, count%128, MEM_SIZE);
		send(sockets[1], buf, MEM_SIZE, 0);
		//~ printf("Sent [%d]\n	-- buf[%d] = %c\n", count, count, buf[count]);

		recv(sockets[1], buf_, 4, 0);
		if (strcmp(buf_, "ack") != 0) {
			perror("Transfer error");
			exit(1);
		}
		count++;
	}
}

int
main(int argc, char** argv) {
	time_t start_t, end_t;

	printf("Socketpair testing program\n\n");
	time(&start_t);
	printf("Started time : %s", ctime(&start_t));
	
	if (socketpair(AF_UNIX, SOCK_STREAM, 0, sockets) < 0) {
		perror("socketpair() error!");
		exit(1);
	}
	
	int pid;
	if ( (pid = fork()) < 0) {
		perror("fork() error!");
		return -1;
	}
	
	if (pid != 0) {		/* parent process */
		close(sockets[0]);
		send_data(TIMES);
	} else {
		close(sockets[1]);
		get_data(TIMES);
		sleep(2);
		exit(0);
	}
	
	/* parent process here */
	time(&end_t);
	printf("Ended time   : %s", ctime(&end_t));
	printf("Ran in %0.0f seconds\n\n", difftime(end_t, start_t));

	printf("Done!\n");
	sleep(1);
	return 0;
}
