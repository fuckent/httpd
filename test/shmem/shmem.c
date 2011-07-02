#include "shmem.h"

int sockets[2];
void * shmem;
sem_t*  semfd;
void
get_data(int nCount) {
	sleep(1);
	sem_t * sem = sem_open("/httpd_sem", O_CREAT, 00700, 0);
	sem_t * acksem = sem_open("/httpd_ack_sem", O_CREAT, 00700, 0);
	if (sem == NULL || acksem == NULL) {
		perror("sem_open failed");
		exit(1);
	}

	char * buf = malloc(MEM_SIZE);
	if (buf == NULL) exit(2);
	int count = 0;
	
	while (count < nCount) {
		sem_wait(sem);
		memcpy(buf, shmem, MEM_SIZE);
		int c = count % 128;
		if (buf[MEM_SIZE-10*count] != c) {
			printf("MEM SIZE is %d\n", MEM_SIZE);
			printf("Received error  buf[%d]=%d\n", MEM_SIZE-10*count, buf[MEM_SIZE-10*count]);
			exit(1);
		}
		//~ printf("got %d\n", count );
		sem_post(acksem);
		count ++;
	}
	sem_close(sem);
	sem_close(acksem);
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
	
	char * buf = malloc(MEM_SIZE); 
	if (buf == NULL)
		{exit(1);}
	
	int count = 0;
	while (count < nCount) {
		memset(buf, count%128, MEM_SIZE);
		memcpy(shmem, buf, MEM_SIZE);
		//printf("+++count %d---%d\n --- %d", count, ((char*)shmem)[1], buf[1]);
		//~ printf("sent %d\n", count );		
		sem_post(sem);
		sem_wait(acksem);
		
		count++;
	}
}


int
main(int argc, char** argv) {
	time_t start_t, end_t;

	printf("Shmem testing program\n\n");
	time(&start_t);
	printf("Started time : %s", ctime(&start_t));
	
	sem_unlink("httpd_sem");
	sem_unlink("httpd_ack_sem");
	shm_unlink("/httpd_shmem");
	
	int shmemfd = shm_open("/httpd_shmem", O_RDWR | O_CREAT, 00700);

	if (shmemfd == -1) {
		perror("shm_open failed");
		exit(1);
	}

	ftruncate(shmemfd, MEM_SIZE);
	if (fork() == 0){
		shmem = mmap(NULL, MEM_SIZE, PROT_READ, MAP_SHARED, shmemfd, 0);
		get_data(TIMES);
		exit(0);
	}

	shmem = mmap(NULL, MEM_SIZE, PROT_WRITE, MAP_SHARED, shmemfd, 0);
	send_data(TIMES);
	
	/* parent process here */
	time(&end_t);
	printf("Ended time   : %s", ctime(&end_t));
	printf("Ran in %0.0f seconds\n\n", difftime(end_t, start_t));
	
	printf("Done!\n");
	sleep(1);
	close(shmemfd);
	return 0;
}
