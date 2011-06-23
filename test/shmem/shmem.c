#include "shmem.h"

int sockets[2];
void * shmem;
sem_t*  semfd;
void
get_data(int nCount) {
	sem_t * sem = sem_open("/httpd_sem", O_CREAT, 00700, 0);
	sem_t * acksem = sem_open("/httpd_ack_sem", O_CREAT, 00700, 0);
	if (sem == NULL || acksem == NULL) {
		perror("sem_open failed");
		exit(1);
	}
	/*
	char str[100];
	sem_wait(sem);
	strcpy(str, shmem);
	printf("Recv: %s\n", str);
	*/
	char * buf = malloc(1024*1024);
	if (buf == NULL) exit(2);
	int count = 0;
	
	while (count < nCount) {
		sem_wait(sem);
		memcpy(buf, shmem, 1024*1024);
		/*int l = 0;
		while ( l != 1024*1024){
			l += recv(sockets[0], buf + l, 1024*1024, 0);

		}*/
		//~ printf("l= %d\n", l);		

		if (buf[1024*1024-10*count] != count%128) {
			printf("Received error  buf[%d]=%c\n", count, buf[count]);
			exit(1);
		}
		sem_post(acksem);
		// send(sockets[0], "ack", 4, 0);
		//printf("Received [%d]\n", count);
		count ++;
	}
}

void
send_data(int nCount) {
	sem_t * sem = sem_open("/httpd_sem", O_CREAT, 00700, 0);
	sem_t * acksem = sem_open("/httpd_ack_sem", O_CREAT, 00700, 0);
	if (sem == NULL)
	{
		perror("sem_open failed");
		exit(1);
	}
	/*
	strcpy(shmem, "Kia chu la chu ech con, co hai la hai mat tron");
	sleep(20);
	sem_post(sem);
	*/
	
	char * buf = malloc(1024*1024); 
	if (buf == NULL)
		{exit(1);}
	
	int count = 0;
	//char buf_[10];
	while (count < nCount) {
		memset(shmem, count%128, 1024*1024);
		(char*)(shmem)[1024*1024-10*count] = count % 128;
		// memcpy(shmem, buf, 1024*1024);
		sem_post(sem);
		sem_wait(acksem);
		
		//send(sockets[1], buf, 1024*1024, 0);
		//~ printf("Sent [%d]\n	-- buf[%d] = %c\n", count, count, buf[count]);

		/*recv(sockets[1], buf_, 4, 0);
		if (strcmp(buf_, "ack") != 0) {
			perror("Transfer error");
			exit(1);
		}*/
		count++;
	}
}


int
main(int argc, char** argv) {
	timer_t start_t, end_t;

	printf("Socketpair testing program\n\n");
	time((time_t*)&start_t);
	printf("Started time : %s", ctime((time_t*)&start_t));
	
	//semfd = sem_open("/sem", O_CREAT);
	
	//shm_unlink("/httpd/shmem");
	int shmemfd = shm_open("/httpd_shmem", O_RDWR | O_CREAT, 00700);

	if (shmemfd == -1) {
		perror("shm_open failed");
		exit(1);
	}

	ftruncate(shmemfd, 1024*1024);
	if (fork() == 0){
		shmem = mmap(NULL, 1024*1024, PROT_READ, MAP_SHARED, shmemfd, 0);
		printf("pointer #2: %p\n", shmem);
		//sleep(2);
		//char str[100];
		//strcpy(str, shmem); //, "Hom qua em toi truong");
		//printf("Recv: %s\n", str);
		get_data(20000);
		exit(0);
	}

	shmem = mmap(NULL, 1024*1024, PROT_WRITE, MAP_SHARED, shmemfd, 0);
	printf("pointer #1: %p\n", shmem);
	//strcpy(shmem, "Hom qua em toi truong");
	send_data(20000);
	
	/* parent process here */
	time((time_t*)&end_t);
	printf("Ended time   : %s\n", ctime((time_t*)&end_t));
	
	printf("Done!\n");
	sleep(1);
	close(shmemfd);
	return 0;
}
