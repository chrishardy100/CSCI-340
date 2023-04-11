#include <ctype.h>
#include <fcntl.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

// GLOBALS ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
int qsize = 0;    // ready_queue size
int sm_size = 0;  // shared_memory size
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
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
void send(struct miniPCB, struct miniPCB*, int);
struct miniPCB recv(struct miniPCB*);
void* scheduler(void*);
void* logger(void*);
int sum(int, int);
int product(int, int);
int power(int, int);
int fibonacci(int, int);
void results(struct miniPCB*);
void printPCB(struct miniPCB);
void print_queue(struct miniPCB*);  // D.0  = N.0.T - M.0.V.E
void read_file(FILE*, struct miniPCB*, char**);
void FCFS(struct miniPCB*);
void SJF(struct miniPCB*);
void PRIORITY(struct miniPCB*);
void write_to_file(struct miniPCB*);
const char* pcbToString(struct miniPCB);
int (*funct_array[4])(int x, int y) = {sum, product, power,
                                       fibonacci};  // Funct & Sched Array
void (*sched_array[3])(struct miniPCB*) = {FCFS, SJF, PRIORITY};

// HEADER FUNCTIONTIONS ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void send(struct miniPCB data, struct miniPCB* queue, int size) {
  // Start Critical Section
  pthread_mutex_lock(&mutex);

  queue[size] = data;
  sm_size++;

  // End Critical Section
  pthread_mutex_unlock(&mutex);
}
struct miniPCB recv(struct miniPCB* queue) {
  // Start Critical Section
  pthread_mutex_lock(&mutex);
  struct miniPCB data = queue[0];
  for (int i = 0; i < 15; i++) queue[i] = queue[i + 1];
  sm_size--;
  // End Critical Section
  pthread_mutex_unlock(&mutex);
  return data;
}

char* set(char* arg) {
  // Fill function & scheduler arrays;

  if (!strcmp(arg, "FCFS")) sched = 0;
  if (!strcmp(arg, "SJF")) sched = 1;
  return arg;
}
int sum(int x, int y) {
  int sum = x + y;
  // printf("SUM = %i\n", sum);
  return sum;
}
int product(int x, int y) {
  int product = x * y;
  // printf("PRODUCT = %i\n", product);
  return product;
}
int power(int x, int y) {
  int power = 1;
  for (int i = 0; i < y; i++) power = power * x;
  // printf("POWER = %i\n", power);
  return power;
}
int fibonacci_recursive(int n) {
  if (n <= 1) return n;
  return fibonacci_recursive(n - 1) + fibonacci_recursive(n - 2);
}
int fibonacci(int x, int y) {
  int result = fibonacci_recursive(y);
  // printf("FIBONACCI: %i\n", result);
  return result;
}
void read_file(FILE* file, struct miniPCB PCB[], char* argv[]) {
  struct miniPCB data;
  char line[20];
  int line_number = 0;
  char ch = fgetc(file);
  int size = 0;
  while (ch != EOF) {
    // Extract line
    for (int i = 0; ch != '\n' && ch != EOF; i++) {
      line[i] = ch;
      ch = fgetc(file);
    }
    // Fill in PCB struct from line
    char* token = strtok(line, ",");
    data.pnum = atoi(token);
    if (!strcmp(argv[1], "SJF")) {
      token = strtok(NULL, ",");
      data.CPUburst = atoi(token);
    } else if (!strcmp(argv[1], "PRIORITY")) {
      token = strtok(NULL, ",");
      data.priority = atoi(token);
    }
    token = strtok(NULL, ",");
    strcpy(data.string, token);
    token = strtok(NULL, ",");
    data.param1 = atoi(token);
    token = strtok(NULL, "\n");
    data.param2 = atoi(token);
    // Create function pointer
    if (!strcmp(data.string, "sum"))
      data.funct = funct_array[0];
    else if (!strcmp(data.string, "product"))
      data.funct = funct_array[1];
    else if (!strcmp(data.string, "power"))
      data.funct = funct_array[2];
    else if (!strcmp(data.string, "fibonacci"))
      data.funct = funct_array[3];
    PCB[data.pnum - 1] = data;
    ch = fgetc(file);
  }
}
void SJF(struct miniPCB* ready_queue) {
  struct miniPCB key;
  struct miniPCB queue[20];
  for (int i = 0; i < qsize; i++) queue[i] = ready_queue[i];
  for (int i = 1; i < qsize; i++) {
    key = queue[i];
    int j = i - 1;
    while (j >= 0 && queue[j].CPUburst > key.CPUburst) {
      queue[j + 1] = queue[j];
      j = j - 1;
    }
    queue[j + 1] = key;
  }
  for (int i = 0; i < qsize; i++) send(queue[i], shared_memory, i);
}
void FCFS(struct miniPCB ready_queue[]) {
  for (int i = 0; i < 15; i++) {
    send(ready_queue[i], shared_memory, i);
  }
}
void PRIORITY(struct miniPCB* ready_queue) {
  struct miniPCB key;

  struct miniPCB queue[20];
  for (int i = 0; i < qsize; i++) queue[i] = ready_queue[i];

  for (int i = 1; i < qsize; i++) {
    key = queue[i];

    int j = i - 1;
    while (j >= 0 && queue[j].priority > key.priority) {
      queue[j + 1] = queue[j];
      j = j - 1;
    }
    queue[j + 1] = key;
  }
  for (int i = 0; i < qsize; i++) send(queue[i], shared_memory, i);
}
void printPCB(struct miniPCB PCB) {
  printf("%i, ", PCB.pnum);
  if (sched == 1) printf("%i ", PCB.CPUburst);
  if (sched == 2) printf("%i ", PCB.priority);

  printf("%s, ", PCB.string);
  printf("%i, ", PCB.param1);
  printf("%i, ", PCB.param2);
  printf("%i\n", PCB.result);
}
void print_queue(struct miniPCB PCB[]) {
  for (int i = 0; i < 15; i++) {
    printPCB(PCB[i]);
  }
}

// THREAD FUNCTIONS ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void* scheduler(void* arg) {
  struct miniPCB* ready_queue = (struct miniPCB*)arg;

  // if (!strcmp("FCFS",arg));
  //   sched_array[0](ready_queue);
  // if(!strcmp(arg, "SJF"))
  //   sched_array[1](ready_queue);
  sched_array[sched](ready_queue);

  pthread_exit(NULL);
}
void* logger(void* arg) {
  // printQueue(shared_memory);
  for (int i = 0; i < qsize; i++) {
    // printPCB(recv(shared_memory));
    recv(shared_memory);
  }
  pthread_exit(NULL);
}

// MAIN THREAD ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

int main(int argc, char* argv[]) {
  argv[1] = set("FCFS");

  funct_array[0] = sum;
  funct_array[1] = product;
  funct_array[2] = power;
  funct_array[3] = fibonacci;
  sched_array[0] = FCFS;
  sched_array[1] = SJF;
  sched_array[2] = PRIORITY;
  ready_queue = (struct miniPCB*)malloc(15 * sizeof(struct miniPCB));
  shared_memory = (struct miniPCB*)malloc(15 * sizeof(struct miniPCB));

  FILE* file;
  if (!strcmp(argv[1], "FCFS"))
    file = fopen("FCFS.txt", "r");
  else if (!strcmp(argv[1], "SJF"))
    file = fopen("SJF.txt", "r");
  else if (!strcmp(argv[1], "PRIORITY"))
    file = fopen("PRIORITY.txt", "r");
  read_file(file, ready_queue, argv);
  //print_queue(ready_queue);

  // THREADS
  pthread_t tid1;
  pthread_t tid2;
  pthread_create(&tid2, NULL, scheduler, ready_queue);
  pthread_create(&tid1, NULL, logger, NULL);
  pthread_join(tid1, NULL);
  pthread_join(tid2, NULL);
}
