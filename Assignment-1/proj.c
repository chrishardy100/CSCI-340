// Nahian Choudhury
// Compilation command: gcc NChoudhury_prj1_sect25_src.c -o NChoudhury_prj1_sect25.exe
// Execution command: ./NChoudhury_prj1_sect25.exe [sourceFileName.txt] [targetFileName.txt]
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>

#define SIZ 128

int main(int argc, const char * argv[]) {
    
    // fd[0] is the read side of pipe & fd[1] is the write side of pipe
    int f1, f2, fd[2]; 
    char line[SIZ];
	
    // Do we have right number of arguments?
    if(argc != 3) {
        char myStr[] = "Wrong number of command line arguments\n";
        write(STDOUT_FILENO, myStr, strlen(myStr));
        return 1;
    }
	
    // Can we access the source file?
    if((f1 = open(argv[1], O_RDONLY, 0)) == -1) {
        char myStr[] = "Can't open source file\n";
        write(STDOUT_FILENO, myStr, strlen(myStr));
        return 2;
    }
	
    // Can we create the target file?
    if((f2 = creat(argv[2], 0644)) == -1) {
        char myStr[] = "Can't create target file\n";
        write(STDOUT_FILENO, myStr, strlen(myStr));
        return 3;
    }
 
    // Can we create the pipe?
    if(pipe(fd) == -1) {
        char myStr[] = "Failed to create pipe\n";
        write(STDOUT_FILENO, myStr, strlen(myStr));
        return 4;
    }
    
    int pID = fork();
    if(pID == -1) {
    	char myStr[] = "Failed to fork\n";
        write(STDOUT_FILENO, myStr, strlen(myStr));
    	return 5;
    }
    	
    if(pID == 0) {
        // Child process
    	// Copy source file contents from pipe to target file. 
		
		close(fd[1]); // Close write end of pipe
		char buf[SIZ];
		int n;
		
		// Reading data from pipe one character at a time until buffer is full then writing buffer to file 
		// Where there is nothing left to be read from the pipe read() will return 0, which will break the loop
		// If there is an interuption during the reading process, read() will return -1, which will also break the loop
        while((n = read(fd[0], line, SIZ)) > 0) {
            write(STDOUT_FILENO, line, n); // Prints to terminal
			// If the write() function is interupted it will return the number of bytes written, which will not != n, causing a break in the loop
            if (write(f2, line, n) != n) {
                char myStr[] = "Can't write to file";
                write(STDOUT_FILENO, myStr, strlen(myStr));
                close(f1);
                close(f2);
                return 6;
            }
        }
        write(STDOUT_FILENO, "\n", strlen("\n")); // Line break in terminal
        close(fd[0]); // Close read end of pipe
		
    } else {
    	// Parent process
		// Copy content from source file to pipe
 	
        close(fd[0]); // Close read end of pipe
        char buf[SIZ];
        int n;
		
		// Reading data from source file one character at a time until buffer is full then writing buffer to pipe 
		// Where there is nothing left to be read from the file read() will return 0, which will break the loop
		// If there is an interuption during the reading process, read() will return -1, which will also break the loop
        while((n = read(f1, buf, SIZ)) > 0) {
			// If the write() function is interupted it will return the number of bytes written, which will not != n, causing a break in the loop
            if(write(fd[1], buf, n) != n) {
                char myStr[] = "Can't write to pipe";
                write(STDOUT_FILENO, myStr, strlen(myStr));
                close(f1);
                close(f2);
                return 7;
            }
        }        
        close(fd[1]); // Close write end of pipe        
    }
 
    return 0;
}