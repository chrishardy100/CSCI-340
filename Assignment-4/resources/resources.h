




// (DONE)TODO: gcc project.c -o project.exe -pthread -fno-stack-protector
/*
 * ---- NOTES
 * Threads share all global variables; the memory space where global variables are stored is shared by all threads
 *  ~ but you still need to pass the refernce thoe
 * 
 * (thread independance)inside your thread function try access global var without reference
 *  ~ you need that feature or else every thread can over write other thread names since they all will have same variables and stuff
 * 
 * fixed it, it wasn't overflow issue but an issue with concatnating two char* into one. 
 *  ~ To fixed it i just used fprintf(ptr, "result %s %s", buff, buff); to write directly to the outfile
 * 
 * 
 * 
 * ----- EXCEPTIONS
 * //  open file exception
 * // arguments exception
 * // thread creation exception
 * // malloc()
 * if(inpArray == NULL){
 *  printf("memory not avalibe");
 *  exit(1);
 * }
 * //
 * //
 * //
 * // thread attribute exception
 * if (s != 0) handle_error_en(s, "pthread_getattr_np"); 
 * 
 * 
 * 
 * 
 * 
 * 
 * [] 1 main program thread, 16 worker child pthreads, and 48 worker computation pthreads1 main program thread, 16 worker child pthreads, and 48 worker computation pthreads
 * [] main program open and read the input file into a 16,000 element int array (InpArray) 
 * [] each thread handles 1000 elements
 *   [] each thread creates 3 child threads and wait's for result from children and passed on to parent using global arrays and die
 *     [] compute geometric average child 1
 *     [] compute arithmetic average, child 2
 *     [] sum for its 1,000 number child 3
 *     [] after all 16 terminated -> Main compute
 *       [] a) The maximum of the 16 geometric average numbers
 *       [] b) The maximum of the 16 arithmetic average numbers
 *       [] c) The maximum of the 16 sum numbers
 *     [] main program write to file for each thread write
 * Worker Child Pthread Number = THREAD_NUMBER : \t Geometric Average = GEOMETRIC_AVERAGE \t Arithmetic Average = ARITHMETIC_AVERAGE \t Sum = SUM \n
 * 
 * 
 * 
 * SUM = 492004
AVERAGE = 246002
ROOT = 0
SUM = 506055
AVERAGE = 253027
ROOT = 0
SUM = 511624
AVERAGE = 255812
ROOT = 0
SUM = 507018
AVERAGE = 253509
ROOT = 0
SUM = 498942
AVERAGE = 249471
ROOT = 0
SUM = 502295
AVERAGE = 251147
ROOT = 0
SUM = 497431
AVERAGE = 248715
ROOT = 0
SUM = 494182
AVERAGE = 247091
ROOT = 0
SUM = 506765
AVERAGE = 253382
ROOT = 0
SUM = 496443
AVERAGE = 248221
ROOT = 0
 * 
 * 
 * 
*/