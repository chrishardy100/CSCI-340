// Christopher Hardy

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <signal.h>
#include <unistd.h>
#define CHUNK_SIZE 10
#define BUFFER_SIZE 40
void child_handler(int sig){
	wait(NULL);
}

struct shared_memory{
	char buff[CHUNK_SIZE];
	int in;
	int out;
};

int main() {
	struct shared_memory *shm_ptr;
	pid_t pid;
	int shm_buff, shm_in, shm_out, shm_fd;
	
	

	const char *name = "OS";
	const char *buffname = "BUFFER";
	const char *inname = "IN";
	const char *outname = "OUT";	
	
	const char *input = "input.txt";
	const char *output = "output.txt";	
	int input_fd = open(input, O_RDWR);
	int output_fd = open(output,  O_RDWR);
	
	

	char *buffptr;
	char *inptr; 
	char *outptr;
	
	shm_fd = shm_open(name, O_RDWR, 0666);
	ftruncate(shm_fd, CHUNK_SIZE * BUFFER_SIZE);
	shm_ptr = mmap(NULL, CHUNK_SIZE * BUFFER_SIZE,  PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);

	shm_buff = open(buffname, O_RDWR, 0666);
	ftruncate(shm_buff, CHUNK_SIZE * BUFFER_SIZE);
	buffptr = (char *)mmap(NULL, BUFFER_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);	
	
	shm_in = open(inname, O_RDWR, 0666);
	ftruncate(shm_in, sizeof(int));
	inptr = mmap(NULL, sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);	

	//shm_out = open(outname, O_RDWR, 0666);
	ftruncate(shm_out, sizeof(int));
	outptr = mmap(NULL, sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);	
	
	
	
	

	pid = fork();
	signal(SIGCHLD, child_handler);							/* rember -pid exception */
	if( pid == 0 ){ 		/* child process */
		while(*inptr != *outptr){
		
		write(output_fd, buffptr, CHUNK_SIZE);
		outptr -= CHUNK_SIZE;
		}
		
	}
	else if( pid > 0 ){	 /* parent process */
		while(((*inptr + 1) % BUFFER_SIZE) != *outptr){
		read(input_fd, buffptr , CHUNK_SIZE);
		inptr -= CHUNK_SIZE;
		}
	
	}
	
	return 0;
}


























//printf("CONSUMER\nPID = %d\n", getpid());
// printf("PRODUCER\nPID = %d\n", getpid());