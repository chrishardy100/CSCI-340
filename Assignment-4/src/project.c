/* Christopher Hardy
 * gcc project.c -o project.exe -pthread -lm
 * ./project.exe
 * 
*/ 
#include <fcntl.h>
#include <ctype.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h> 
#include <string.h>
#include <unistd.h>
#include <math.h>
#define FILE_SIZE 10000  // integers in FILE file
#define C_THREADS 10     // number of child threads
#define GC_THREADS 3     // number of grandchild threads

// FUNCTION DECLARATIONS 
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void read_from_file(FILE*, int*);  // reads from selected file and puts contents into an integer array.
void* worker_threads(void*);      
void* arithmetic_avreage_thread(void*);
void* adder_thread(void*);      
void* root_of_sum_of_squares_thread(void*);
int find_min_value(int*);
void write_to_file(int*, int*, int*);
int *mallocArray;
// MAIN THREAD 
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
int main(int argc, char* argv[]) { 
  FILE* file = fopen("INPUT.txt", "r");  // open file (initialize & declare file decriptor).
  mallocArray = (int*)malloc(sizeof(int)*GC_THREADS);
  // (inpArray) stores contents of INPUT.txt, (results) to store return values from threads, arrays to store results
  int inpArray[FILE_SIZE], *results;
  int aveArray[C_THREADS], sumArray[C_THREADS], rssArray[C_THREADS];
  read_from_file(file, inpArray);  // read from file (declares inpArray).

  pthread_t thread_id[C_THREADS];  // array of thread IDs to store the 10 worker threads ids
  for(int i=0; i < C_THREADS; i++) { // loop to create 10 worker threads.
    pthread_create(&thread_id[i], NULL, worker_threads, &inpArray[(FILE_SIZE/C_THREADS)*i]);  // worker_thread creation
    pthread_join(thread_id[i],(void**) &results); // Each of the 10 worker thread returns an array of results(results).
    aveArray[i] = results[0];   // results[0] is stored in an array of 10 arithmetic averages
    sumArray[i] = results[1];   // results[1] is stored in an array of 10 sums
    rssArray[i] = results[2];   // results[2] is stored in an array of 10 roots of sums of squares
  } 
  write_to_file(aveArray, sumArray, rssArray); // Produces the output file
  free(mallocArray);  // Free dynamically allocated memory.
  return 0; // Exit the main thread.
}

// CHILD THREAD FUNCTIONS 
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void* worker_threads(void* arg){
  int *array = (int *)arg;; // Stores the arguments passed from pthread_create().
  int *average, *sum, *rss; // Variables to store the results of the computation threads.
  int results[GC_THREADS];  // An array that will be used to return results back to the main thread.
  // print_thread(array);
  pthread_t comp_thread_id[GC_THREADS];  // array of 3 thread IDs for computation threads.
  pthread_create(&comp_thread_id[0], NULL, arithmetic_avreage_thread, arg);  // Creates thread for aritmetic average.
  pthread_create(&comp_thread_id[1], NULL, adder_thread, arg);  // Creates thread for sum.
  pthread_create(&comp_thread_id[2], NULL, root_of_sum_of_squares_thread, arg);  // Creates thread for root sum squares.
  
  pthread_join(comp_thread_id[0], (void**) &average); // thread joins 
  pthread_join(comp_thread_id[1], (void**) &sum);     // thread joins
  pthread_join(comp_thread_id[2], (void**) &rss);     // thread joins
  results[0]  = *average; // Average is the first value in the array to be returned to main thread.
  results[1] = *sum;      // Sum is the second value
  results[2] = *rss;      // Root of Sum of Squares is the third value.
  *mallocArray = *results;  // Assign the results array to the dynamically allocated memory
  return (void*) mallocArray; // return a pointer to the dynamically allocated memory.
  pthread_exit(0); // wait()
}

// GRAND CHILD THREAD FUNCTIONS 
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void* arithmetic_avreage_thread(void* arg){
  int *array = (int *)arg;  // Stores the arguments passed from pthread_create().
  int sum = 0, average; // Variable to hold the value of the sum, the divisor(n), and the return value(average).  
  for(int i = 0; i < FILE_SIZE/C_THREADS; i++) // FILE_SIZE / C_THREADS = number of elements given to each thread(1000)
    sum += array[i];  // Sum of all the numbers of a subset.
  average = sum / (FILE_SIZE/C_THREADS);  // Aritmetic average is the sum of all the elements divided by the total number of elements.
  mallocArray[0] = average; // Place average result into dynamically allocated memory to be later returned to the main thread.
  return (void*) (mallocArray); // return a pointer to the dynamically allocated memory.
  pthread_exit(0); // wait()
}
void* adder_thread(void* arg){
  int *array = (int *)arg;  // Stores the arguments passed from pthread_create().
  int sum = 0;
  for(int i = 0; i < FILE_SIZE/C_THREADS; i++)
    sum += array[i];  // Add togeather all the elements in a array.
  mallocArray[1] = sum; // Place sum result into dynamically allocated memory to be later returned to the main thread.
  return (void*) (mallocArray+1); // return a pointer to the dynamically allocated memory.
  pthread_exit(0); // wait()
}
void* root_of_sum_of_squares_thread(void* arg){
  int *array = (int *)arg;  // Stores the arguments passed from pthread_create().
  int sum = 0, rss;
  for(int i = 0; i < FILE_SIZE/C_THREADS; i++)
    sum += array[i] * array[i]; // Add togeather all entries squared.
  rss = sqrt(sum);  // The final result is the square root of the sum of all entries squared.
  mallocArray[2] = (int)rss;  // Place root sum square result into dynamically allocated memory to be later returned to the main thread.
  return (void*) (mallocArray+2); // return a pointer to the dynamically allocated memory.
  pthread_exit(0); // wait()
}

// AUXILIARY FUNCTIONTIONS 
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void write_to_file(int* aveArray, int* sumArray, int* rssArray){
  int file = open("OUTPUT.txt", O_RDWR);  // open file (initialize & declare file decriptor).
  int buffSize = 50;  // Size of chracter strings that output reults.
  char threadNumChar[buffSize], aveChar[buffSize], sumChar[buffSize], rssChar[buffSize];
  char minAveChar[buffSize], minSumChar[buffSize], minRssChar[buffSize];
  for(int i = 0; i < C_THREADS; i++){
    sprintf(threadNumChar, "THREAD_NUMBER = %d :\t", i);        // Take thread number and place it into a string(threadNumChar).
    sprintf(aveChar, "ARITHMETIC_AVERAGE = %d\t", aveArray[i]); // Take aritmetic average result and place it into a string(aveChar).
    sprintf(sumChar, "SUM = %d\t", sumArray[i]);                // Take sum result and place it into a string(sumChar).
    sprintf(rssChar, "ROOT_SUM_SQUARES = %d\n", rssArray[i]);   // Take root sum square result and place it into a string(rssChar).
    write(file, threadNumChar, strlen(threadNumChar));  // Writes thread number to selected output file.
    write(file, aveChar, strlen(aveChar));              // Writes aritmetic average result to selected output file.
    write(file, sumChar, strlen(sumChar));              // Writes sum result to selected output file.
    write(file, rssChar, strlen(rssChar));              // Writes root sum square result to selected output file.
  }
  
  int minAve = find_min_value(aveArray);  // Returns the minimum aritmetic average value in aveArray.
  int minSum = find_min_value(sumArray);  // Returns the minimum sum value in sumArray.
  int minRss = find_min_value(rssArray);  // Returns the minimum root sum square value in rssArray.


  sprintf(minAveChar, "MIN_ARITHMETIC_AVERAGE  = %d\n", minAve); // Take the minimum aritmetic average result and place it into a string(minAveChar).
  sprintf(minSumChar, "MIN_SUM = %d\n", minSum);  // Take the minimum sum result and place it into a string(minSumChar).
  sprintf(minRssChar, "MIN_OF_ROOT_SUM_SQUARES = %d\n", minRss);  // Take the minimum root sum square result and place it into a string(minRssChar).
  write(file, minAveChar, strlen(minAveChar));  // Writes minimum aritmetic average result to selected output file.
  write(file, minSumChar, strlen(minSumChar));  // Writes minimum sum result to selected output file.
  write(file, minRssChar, strlen(minRssChar));  // Writes minimum root sum square result to selected output file.
}
int find_min_value(int* array){
  int minValue = array[0];  // Function that returns the minimum value in a given input array
  for(int i = 1; i < C_THREADS; i++){
    if(array[i] < minValue) minValue = array[i];
  }
  return minValue;
}
void read_from_file(FILE* file, int* array){  
  for(int i = 0; i < FILE_SIZE; i++){ // Function that reads from an input file and places its contents into an array.
    fscanf(file, "%i", array + i);
  }
}


