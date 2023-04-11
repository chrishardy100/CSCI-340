
#include <pthread.h>
#include <stdio.h>
#define NUM_LOOPS 20

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

long long sum = 0;

void* counter(void* arg) {
  int offset = *(int*)arg;
  for (int i = 0; i < 20; i++) {
    // Start critical section
    pthread_mutex_lock(&mutex);
    sum += offset;
    pthread_mutex_unlock(&mutex);
    // End critical section
  }
  pthread_exit(NULL);
}

int main(int argc, char** argv) {
  // pthread 1
  pthread_t id1;
  int offset1 = 2;
  pthread_create(&id1, NULL, counter, &offset1);

  // pthread 2
  pthread_t id2;
  int offset2 = -1;
  pthread_create(&id2, NULL, counter, &offset2);

  pthread_join(id1, NULL);
  pthread_join(id2, NULL);

  printf("SUM = %lld\n", sum);
}