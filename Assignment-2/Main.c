// Christopher Hardy
// gcc HARDY_23926681.c -o HARDY_23926681.exe -pthread
#include <fcntl.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#define SIZE 15

// GLOBAL ATTRIBUTES ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
int sched;
struct miniPCB {
  int pnum;
  int CPUburst;
  char string[10];
  int param1;
  int param2;
  int priority;
  int (*funct)(int param1, int param2);
  int result;
};
int (*funct_array[4])(int x, int y);  // Funct & Sched Array
void (*sched_array[3])(struct miniPCB*);
struct miniPCB* shared_memory;
struct miniPCB* ready_queue;
pthread_mutex_t mutex;

// AUX FUNCTIONTIONS ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void printPCB(struct miniPCB PCB) {
  // Prints the content of any PCB
  printf("%i, ", PCB.pnum);
  // Check scheduler
  if (sched == 1) printf("%i ", PCB.CPUburst);
  if (sched == 2) printf("%i ", PCB.priority);
  printf("%s, ", PCB.string);
  printf("%i, ", PCB.param1);
  printf("%i, ", PCB.param2);
  printf("%i\n", PCB.result);
}
void print_queue(struct miniPCB PCB[]) {
  // Prints all PCBs in a queue
  for (int i = 0; i < SIZE; i++) {
    printPCB(PCB[i]);
  }
}
void send(struct miniPCB data, struct miniPCB* queue, int size) {
  // Start Critical Section
  pthread_mutex_lock(&mutex);

  queue[size] = data;

  // End Critical Section
  pthread_mutex_unlock(&mutex);
}
struct miniPCB recv(struct miniPCB* queue) {
  struct miniPCB data = queue[0];
  // Start Critical Section
  pthread_mutex_lock(&mutex);
  // print_queue(ready_queue);
  for (int i = 0; i < 15; i++) queue[i] = queue[i + 1];
  // End Critical Section
  pthread_mutex_unlock(&mutex);
  return data;
}
int sum(int x, int y) {
  // Sum function funct_array[0]
  return x + y;
}
int product(int x, int y) {
  // Product function funct_array[1]
  return x * y;
}
int power(int x, int y) {
  // Power functio funct_array[2]
  int power = 1;
  for (int i = 0; i < y; i++) power = power * x;
  return power;
}
int fibonacci_recursive(int n) {
  if (n <= 1) return n;
  return fibonacci_recursive(n - 1) + fibonacci_recursive(n - 2);
}
int fibonacci(int x, int y) {
  // Fibonacci function funct_array[3]
  int result = fibonacci_recursive(y);
  return result;
}
void results(struct miniPCB* ready_queue) {
  // Computes the results of the commands in a queue
  for (int i = 0; i < SIZE; i++) {
    int x = ready_queue[i].param1;
    int y = ready_queue[i].param2;
    ready_queue[i].result = ready_queue[i].funct(x, y);
  }
}
void read_file(FILE* file, struct miniPCB PCB[], char* argv[]) {
  // Declare space for next PCB to be stored
  struct miniPCB data;
  // Line buffer
  char line[20];
  char ch = fgetc(file);
  while (ch != EOF) {
    // Extract line
    for (int i = 0; ch != '\n' && ch != EOF; i++) {
      line[i] = ch;
      ch = fgetc(file);
    }
    // Fill in PCB struct from line
    char* token = strtok(line, ",");
    // Process Number
    data.pnum = atoi(token);
    if (!strcmp(argv[1], "SJF")) {
      token = strtok(NULL, ",");
      // CPU burst
      data.CPUburst = atoi(token);
    } else if (!strcmp(argv[1], "PRIORITY")) {
      token = strtok(NULL, ",");
      // Priority
      data.priority = atoi(token);
    }
    token = strtok(NULL, ",");
    // String
    strcpy(data.string, token);
    token = strtok(NULL, ",");
    // Parameter 1
    data.param1 = atoi(token);
    token = strtok(NULL, "\n");
    // Parameter 2
    data.param2 = atoi(token);
    // Assign  function pointer
    if (!strcmp(data.string, "sum"))
      data.funct = funct_array[0];
    else if (!strcmp(data.string, "product"))
      data.funct = funct_array[1];
    else if (!strcmp(data.string, "power"))
      data.funct = funct_array[2];
    else if (!strcmp(data.string, "fibonacci"))
      data.funct = funct_array[3];
    // Place data int the queue
    PCB[data.pnum - 1] = data;
    ch = fgetc(file);
  }
}
void SJF(struct miniPCB* queue) {
  struct miniPCB key;
  struct miniPCB sm_queue[20];
  for (int i = 0; i < SIZE; i++) sm_queue[i] = queue[i];
  for (int i = 1; i < SIZE; i++) {
    key = sm_queue[i];
    int j = i - 1;
    while (j >= 0 && sm_queue[j].CPUburst > key.CPUburst) {
      sm_queue[j + 1] = sm_queue[j];
      j = j - 1;
    }
    sm_queue[j + 1] = key;
  }
  for (int i = 0; i < SIZE; i++) send(sm_queue[i], shared_memory, i);
}
void FCFS(struct miniPCB ready_queue[]) {
  // Places the ready queue directly into the shared queue
  for (int i = 0; i < 15; i++) {
    send(ready_queue[i], shared_memory, i);
  }
}
void PRIORITY(struct miniPCB* queue) {
  // Uses insertion sort to sort the ready queue based on priority
  struct miniPCB key;
  struct miniPCB sm_queue[20];
  for (int i = 0; i < SIZE; i++) sm_queue[i] = queue[i];
  for (int i = 1; i < SIZE; i++) {
    key = sm_queue[i];
    int j = i - 1;
    while (j >= 0 && sm_queue[j].priority > key.priority) {
      sm_queue[j + 1] = sm_queue[j];
      j = j - 1;
    }
    sm_queue[j + 1] = key;
  }
  for (int i = 0; i < SIZE; i++) send(sm_queue[i], shared_memory, i);
}
void write_to_file(struct miniPCB queue[]) {
  ssize_t rd;
  // Declare space for next PCB to be writen
  struct miniPCB data;
  // Open output file
  int output;
  if(sched == 0) 
    output = open("FCFSoutput.txt", O_WRONLY, O_CREAT);
  else if (sched == 1)
    output = open("SJFoutput.txt", O_WRONLY, O_CREAT);
  else if (sched == 2)
    output = open("PRIORITYoutput.txt", O_WRONLY, O_CREAT);
  
    for (int i = 0; i < SIZE; i++) {
      // Receive data from the shared queue
      data = recv(shared_memory);
      // Write command
      if (!strcmp(data.string, "sum"))
        write(output, data.string, 3);
      else if (!strcmp(data.string, "product"))
        write(output, data.string, 7);
      else if (!strcmp(data.string, "power"))
        write(output, data.string, 5);
      else if (!strcmp(data.string, "fibonacci"))
        write(output, data.string, 9);
      write(output, ", ", 2);
      // Write parameter 1
      char ch[2];
      int p1 = data.param1;
      for (int j = 1; j >= 0; j--) {
        ch[j] = p1 % 10 + '0';
        p1 = p1 / 10;
      }
      write(output, &ch, 2);
      write(output, ", ", 2);
      // Write parameter 2
      char ch2[2];
      int p2 = data.param2;
      for (int j = 1; j >= 0; j--) {
        ch2[j] = p2 % 10 + '0';
        p2 = p2 / 10;
      }
      write(output, &ch2, 2);
      write(output, ", ", 2);
      // Write result
      char ch3[5];
      int ans = data.result;
      for (int j = 4; j >= 0; j--) {
        ch3[j] = ans % 10 + '0';
        ans = ans / 10;
      }
      write(output, &ch3, 5);
      write(output, "\n", 1);
    }
  // Sync data and close output file
  fdatasync(output);
  close(output);
}

// THREAD FUNCTIONS ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void* scheduler(void* arg) {
  struct miniPCB* ready_queue = (struct miniPCB*)arg;
  sched_array[sched](ready_queue);
  results(shared_memory);
  pthread_exit(0);
}
void* logger(void* arg) {
  print_queue(shared_memory);
  write_to_file(shared_memory);
  pthread_exit(0);
}

// MAIN THREAD ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

int main(int argc, char* argv[]) {
  // Check arguments to set scheduler
  if (!strcmp(argv[1], "FCFS.txt")) sched = 0;
  else if (!strcmp(argv[1], "SJF.txt")) sched = 1;
  else if (!strcmp(argv[1], "PRIORITY.txt")) sched = 2;
  // sill function & scheduler funtion pointer array
  funct_array[0] = sum;
  funct_array[1] = product;
  funct_array[2] = power;
  funct_array[3] = fibonacci;
  sched_array[0] = FCFS;
  sched_array[1] = SJF;
  sched_array[2] = PRIORITY;
  // Initalize dynamic memory for ready queue and interprocess communication thread

  struct miniPCB* ready_queue = (struct miniPCB*)malloc(15 * sizeof(struct miniPCB));
  shared_memory = (struct miniPCB*)malloc(15 * sizeof(struct miniPCB));
  
  //Open file
  FILE* file;
  if (!strcmp(argv[1], "FCFS"))
    file = fopen("FCFS.txt", "r");
  else if (!strcmp(argv[1], "SJF"))
    file = fopen("SJF.txt", "r");
  else if (!strcmp(argv[1], "PRIORITY"))
    file = fopen("PRIORITY.txt", "r");
  // Read file and put commands in ready_queue
  read_file(file, ready_queue, argv);

  // THREADS
  pthread_t tid1;
  pthread_t tid2;
  pthread_mutex_init(&mutex, NULL);
  pthread_create(&tid2, NULL, scheduler, ready_queue);
  pthread_create(&tid1, NULL, logger, NULL);
  pthread_join(tid1, NULL);
  pthread_join(tid2, NULL);
  pthread_mutex_destroy(&mutex);
}
