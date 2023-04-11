#include <fcntl.h>
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>

void* run(void* arg) {
  int* arg_ptr = (int*)arg;
  int argument = *arg_ptr;
  free(arg);

  long long sum = 0;  // SUM
  for (int i = 0; i <= argument; i++) {
    sum += i;
  }

  int* ans = malloc(sizeof(*ans));
  *ans = sum;
  pthread_exit(ans);
}

int main(int argc, char** argv) {
  // TODO: args & args exception
  int num_args = argc - 1;

  // Dynamic Memory
  int* limit = malloc(sizeof(*limit));
  *limit = atoi(argv[1]);

  // Thread ID:
  pthread_t tids[num_args];

  // Thread attributes:
  pthread_attr_t attr;
  pthread_attr_init(&attr);

  // Thread creation:
  pthread_create(&tids[0], &attr, run, limit);

  int* result;

  pthread_join(tids[0], (void**)&result);
  printf("Sum is %i\n", *result);
  free(result);

  return 0;
}