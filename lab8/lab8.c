/*
 * Willard Wider
 * 03/27/18
 * lab8.c
 * lab on mutexes and semaphores
 * combining lab 7 and 6
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/types.h>
#include <pthread.h>
#include <time.h>
#include <semaphore.h>

#define NUM_CHARS 1000
//#define DEBUG 1

#define BILLION 1E9

//variables for the 
//the semaphores for mutex and waiting for full
sem_t full;
sem_t empty;
//value for debugging. defualt value is 0
int DEBUG = 0;
//keeping track of updating the producer
int in = 0;
//keeping track of updating the consumer
int out = 0;
//variables for keeping track of how much produced and consumed
int total_sent = 0;
int total_recieved = 0;
//the num ints for the for loop
//number of itterations
int numIts = 0;
double total_time = 0.0;
double total_time_nanosec = 0.0;
int threadItts = 0;
int mainItts = 0;
int recieved;
struct timespec start;
struct timespec end;
int counter = 0;
//int to get the method to use
//1=petersons/mutex, 2 = semaphores
int method = 0;
//the actual queue
int queue[NUM_CHARS];
pthread_mutex_t locker = PTHREAD_MUTEX_INITIALIZER;
//the method to incriment and decriment
void *run();

int main(int argc,char* argv[])
{
  //thread and it's attributes
  pthread_t tid;
  if(argv[1] != NULL)
    numIts = atoi(argv[1]);
  if(argv[2] != NULL)
    method = atoi(argv[2]);
  if(argv[3] != NULL)
    DEBUG = atoi(argv[3]);
  if (numIts <= 0)
  {
    printf("invalid number of numIts: %d\n",numIts);
    return -1;
  }
  if(method < 1 || 2 < method)
  {
    printf("inbalid method parameter: %d. valid parameters are\n1=petersons/mutex, 2=semaphores\n", method);
    return -1;
  }
  //init the semaphores
  if(sem_init(&full,1,0))
  {
    printf("failed to create semaphore\n");
    return -1;
  }
  //empty is actually true
  if(sem_init(&empty,1,NUM_CHARS))
  {
    printf("failed to create semaphore\n");
    return -1;
  }
  printf("running %d iterations using method %s...\n",numIts, method == 1? "peterTex" : "semaphores");
  //create the thread and set it to run the run method
  if(pthread_create(&tid,NULL,run,NULL))
  {
    printf("failed to create thread\n");
    return -1;
  }
  //session 0 producer
  if(DEBUG)
    printf("parent: child process created, sending values...\n");
  //make a local to prevent any read locks
  int methodd = method;
  while(1)
  {
    switch(methodd)
    {
      case 1:
        //if the buffer is full do nothing
        while(counter == NUM_CHARS) ;
        queue[in] = total_sent;
        in = (in+1) % NUM_CHARS;
        pthread_mutex_lock(&locker);
        counter++;
        pthread_mutex_unlock(&locker);
        total_sent++;
      break;
      case 2:
        //if the buffer is full do nothing
        //wait empty
        sem_wait(&empty);
        //buffer has at least one spot available, set it
        //to make sure each value is differnet for easy purposes,
        //make the calue the total sent
        queue[in] = total_sent;
        in = (in+1) % NUM_CHARS;
        total_sent++;
        //signal full
        sem_post(&full);
      break;
    }
    if (DEBUG)
      printf("DEBUG: parent sending %d, in=%d,total_sent=%d\n",total_sent-1,in,total_sent);
    if(total_sent >= numIts)
      break;
  }
  //wait for it
  pthread_join(tid,NULL);
  total_time_nanosec = (BILLION*total_time)+total_time_nanosec;
  printf("program done, took the CPU %f total miliseconds\n",total_time_nanosec / 1000000);
  return 0;
}

void *run()
{
  //session 1 consumer
  //get the tme before starting...
  if(clock_gettime(CLOCK_THREAD_CPUTIME_ID, &start) == -1)
  {
    printf("error in clock_gettime\n");
    pthread_exit(0);
  }
  if(DEBUG)
    printf("child: listinging for the string...\n");
  while(1)
  {
    //make a local to prevent any read locks
    int methodd = method;
    switch(methodd)
    {
      case 1:
        //if 0, nothing to recieve...
        while(counter == 0) ;
        recieved = queue[out];
        out = (out+1) % NUM_CHARS;
        pthread_mutex_lock(&locker);
        counter--;
        pthread_mutex_unlock(&locker);
        total_recieved++;
      break;
      case 2:
        //wait full
        sem_wait(&full);
        recieved = queue[out];
        out = (out+1) % NUM_CHARS;
        total_recieved++;
        //signal empty
        sem_post(&empty);
      break;
    }
    if (DEBUG)
      printf("DEBUG: child recieved value: %d, out=%d,total_revieced=%d\n",recieved,out,total_recieved);
    if(total_recieved >= numIts)
      break;
  }
  //calc how long that just took
  //first get the total seconds
  if(clock_gettime(CLOCK_THREAD_CPUTIME_ID, &end) == -1)
  {
    printf("error in clock_gettime\n");
    pthread_exit(0);
  }
  //note that nsec is precision of previous second
  total_time = end.tv_sec - start.tv_sec;
  total_time_nanosec = end.tv_nsec - start.tv_nsec;
  if(total_time_nanosec < 0)
  {
    total_time--;
    total_time_nanosec = BILLION + end.tv_nsec - start.tv_nsec;
  }
  pthread_exit(0);
}
