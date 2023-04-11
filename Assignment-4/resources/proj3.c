// Christopher Hardy 23926681
// gcc HARDY_23926681.c -o HARDY_23926681.exe -lrt
#include <fcntl.h>           /* For O_* constants */
#include <sys/stat.h>        /* For mode constants */
#include <mqueue.h>
#include <stdlib.h>     // atoi()
#include <unistd.h>     // pipe() fork() read() write() close()
#include <sys/wait.h>   // wait()
#define SIZE 15
#define MQ_NAME "mq_queue"

// GLOBAL ATTRIBUTES ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

struct mq_attr attributes = {
  .mq_flags = 0,       /* Flags (ignored for mq_open()) */
  .mq_maxmsg = 10,      /* Max. # of messages on queue */
  .mq_msgsize = sizeof(int),     /* Max. message size (bytes) */
  .mq_curmsgs = 0     // # of messages currently in queue (ignored for mq_open()) 
};

// FUNCTIONS ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
char options_list[4] = {'0'};
char* itocs(char* result, int input){
  int temp = input;
  char char_value;
  char t_result[sizeof(input)];
  for(int i = sizeof(input)-1; temp > 0; i--){
    char_value = (temp%10)+'0';
    t_result[i] = char_value;
    temp = temp/10;
  }
  result = t_result;
  return result;

}

char * options_menu(char* input){
  for(int i = 1; i < sizeof(input); i++){
    char curr_char = input[i];
    if(curr_char == 'l') options_list[0] = curr_char;
    else if(curr_char == 'w') options_list[1] = curr_char;
    else if(curr_char == 'm') options_list[2] = curr_char;
    else if(curr_char == 'L') options_list[3] = curr_char;
  }
  return options_list;
}
int newline_count(char* buffer, int size){ // not complete
  int newlines = 0;
  for(int i = 0; i < size; i++){
    char curr_char = buffer[i];
    if(curr_char == '\n')
      newlines++;   
  }
  return newlines;
}
int word_count(char* buffer, int size){
  int words = 0;
  for(int i = 0; i < size; i++){
    char curr_char = buffer[i];
    if(curr_char == ' ')
      words++; 
  }
  return words;
}
int char_count(char* buffer, int size){
  int chars = 0;
  for(int i = 0; i < size; i++){
    char curr_char = buffer[i];
    chars++;
  }
  return chars;
}
int max_line_length(char* buffer, int size){
  int length = 0;
  for(int i = 0; i < size; i++){
    char curr_char = buffer[i];
  }
  return ++length;
}

// MAIN THREAD ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
int main(int argc, char* argv[]) {
  // VARIABLE-SETUP
  int buf_size;
  char* options = "lwm0"; // default options;  // a ist of options as specified by the command line.
  const char* msg; // a message buffer to be sent trough the queue
  mqd_t queue;  // meesge queue initilization
  int file;

  // ARGUMENTS
  for(int i = 1; i < argc; i++){ // loops through the arguments sarting with argv[1]
    // TODO: check argv[3] for input files
    int curr_char = argv[i][0];
    if(argv[1][0] == '-'){
      options = options_menu(argv[1]);
      file = open(argv[3], O_RDONLY, 0); // opens file
    } 
    else{
      file = open(argv[2], O_RDONLY, 0); // opens file
    }
    if(atoi(argv[1]) != 0) buf_size = atoi(argv[1]); //size of buffer as specified in the arguments
    else buf_size = atoi(argv[2]); //size of buffer as specified in the arguments
  }
  queue = mq_open(MQ_NAME, O_CREAT | O_RDWR, S_IRUSR | S_IWUSR, &attributes);

  // PIPE
  int pfd[2]; // array of file discriptors used by pipe() for reading & writing respectivly.
  if(pipe(pfd) == -1)
    return 1; //ERROR
  int pid = fork(); //forks to create a child process pid=0 and a parent process p>0
  // CHILD PROCESS
  if(pid == 0){  
    close(pfd[1]);  // pfd[0] read
    int size = buf_size; // size of the buffer later chaged by read() if buffer is partially full
    char c_buffer[size];  // child buffer for pfd[0] to fill
    int newlines = 0, words = 0, chars = 0, max_line=0; // tracker variables
    char* newlines_string;
    char* words_string;
    char* chars_string;
    char* max_line_string;
    
    while((size = read(pfd[0], c_buffer, buf_size))>0){
      if(options[0] == 'l'){
        newlines += newline_count(c_buffer, size);  // print # of newlines option
      }
      if(options[1] == 'w')
        words += word_count(c_buffer, size);  // print # of words option
      if(options[2] == 'm')
        chars += char_count(c_buffer, size);  // print # of characters option
      if(options[3] == 'L')
        max_line += max_line_length(c_buffer, size); 
      // write(STDOUT_FILENO, c_buffer, size); // prints contents of buffer
    }
    // newlines_string=itocs(newlines_string, newlines);
    // words_string=itocs(words_string, words);
    // chars_string=itocs(chars_string, chars);
    // max_line_string=itocs(max_line_string, max_line);
    // write(STDOUT_FILENO, newlines_string, sizeof(newlines_string));
    // write(STDOUT_FILENO, "\n", 1);
    // write(STDOUT_FILENO, words_string, sizeof(words_string));
    // write(STDOUT_FILENO, chars_string, sizeof(chars));
    // mq_send(queue, msg, sizeof(int),1);
    close(pfd[0]);
  }
  // PARENT PROCESS
  else { 
    close(pfd[0]); // pfd[1] write
    int char_read; //  number of characters read by the parent process and placed into the write end of pipe[1]
    char p_buffer[buf_size]; // buffer for the parent to write to and then use to write to pipe()
    while((char_read = read(file, p_buffer, buf_size)) > 0){ //Read from file
      write(pfd[1], p_buffer, char_read); // write from pipe to buffer p_buffer
    }
    char* message_recieved;
    // mq_receive(queue, message_recieved, sizeof(int),NULL);
    close(pfd[1]);
    wait((int *)NULL);  // wait for the child process to terminate
  }
  return 1;
}
// TODO: Exception for pipe
// TODO: open file exception
// TODO: mq_create exception
// TODO: 
// TODO: 
// TODO: 
// TODO: 
// TODO: print output of sizes using write() 
// TODO: msg_queue -lrt
// TODO: and pfd[0] is the read end of the pipe while pfd[1] is the write end of the pipe
// TODO: line = realloc(line, sizeof(buffSize));
// TODO: 
// TODO: 
// TODO: 
// TODO: 
// TODO: 
// TODO: 
// TODO: 
// TODO: 
// TODO: 
// (DONE)TODO: BUFFER_SIZE=atoi(argv[2]) n
// (DONE)TODO: ssize_t size =sizeof(buf)/sizeof(char); // whole size / 4 which is size of each char 