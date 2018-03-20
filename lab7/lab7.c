/*
 * Willard Wider
 * 03/19/18
 * lab7.c
 * lab on putersons and mutexes
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

#define DEBUG 1

#define BILLION 1E9

//variables for the petersons method
//whose turn it is to enter the crtiical section
int turn;
//if thread n is ready for entry into the critical process
int ready[2];

//variables for the 
//counter that will be fought over
int counter = 0;
//the num ints for the for loop
//number of itterations
int numIts = 0;
//counter for getting the CPU timing of the second thread
int numUnits = 0;
//int to get the method to use
//1=petersons, 2 = mutex
int method = 0;
double total_time = 0.0;
double total_time_nanosec = 0.0;
int threadItts = 0;
int mainItts = 0;
struct timespec start;
struct timespec end;
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
  if (numIts <= 0)
  {
    printf("invalid number of numIts: %d\n",numIts);
    return -1;
  }
  if(method < 1 || 2 < method)
  {
    printf("inbalid method parameter: %d. valid parameters are\n1=petersons, 2=mutex\n");
    return -1;
  }
  printf("running %d iterations of method %s...\n",numIts, method==1? "peterson":"mutex");
  //create the thread and set it to run the run method
  if(pthread_create(&tid,NULL,run,NULL))
  {
    printf("failed to create thread\n");
    return -1;
  }
  //session 0
  
  for ( ; mainItts < numIts; mainItts++)
  {
    switch(method)
    {
      case 1:
        ready[0] = 1;
        turn = 1;
        while((ready[1] == 1) && (turn==1)) ;
        //parent has entered sensitive code
        counter++;
        ready[0] = 0;
      break;
      case 2:
        pthread_mutex_lock(&locker);
        counter++;
        pthread_mutex_unlock(&locker);
      break;
    }
  }
  //wait for it
  pthread_join(tid,NULL);
  printf("program done, value of counter is %d\ntook the CPU %f second, %f nanoseconds\n%f total nanoseconds\n"
    ,counter,total_time,total_time_nanosec,(BILLION*total_time)+total_time_nanosec);
  return 0;
}

void *run()
{
  //session 1
  //get the tme before starting...
  if(clock_gettime(CLOCK_REALTIME, &start) == -1)
  {
    printf("error in clock_gettime\n");
    pthread_exit(0);
  }
  for ( ; threadItts < numIts; threadItts++)
  {
    switch(method)
    {
      case 1:
        ready[1] = 1;
        turn = 0;
        while((ready[0] == 1) && (turn==0)) ;
        //child has entered the sensitive code
        counter--;
        ready[1] = 0;
      break;
      case 2:
        pthread_mutex_lock(&locker);
        counter--;
        pthread_mutex_unlock(&locker);
      break;
    }
  }
  if(clock_gettime(CLOCK_REALTIME, &end) == -1)
  {
    printf("error in clock_gettime\n");
    pthread_exit(0);
  }
  //calc how long that just took
  //first get the total seconds
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
