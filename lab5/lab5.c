/*
 * Willard Wider
 * 02/23/18
 * lab5.c
 * lab on threading and race conditions
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

//counter that will be fought over
int counter = 0;
//the num ints for the for loop
//number of itterations
int numIts = 0;
//counter for the for loop in the thread
int threadItts;
int mainItts;
//the method to incriment and decriment
void *run();

int main(int argc,char* argv[])
{
  //thread and it's attributes
  pthread_t tid;
  //argv[0] is the program name
  //printf("program name is: %s\n",argv[0]);
  if(argc != 2)
  {
    printf("must supply 1 parameter of type int\n");
    return -1;
  }
  numIts = atoi(argv[1]);
  if (numIts < 1)
  {
    printf("invalid number of iterations\n");
    return -1;
  }
  mainItts = 0;
  printf("requesting %d iterations...\n", numIts);
  pthread_create(&tid,NULL,run,NULL);
  //wait for it...
  //while we'ere waiting, let's incriment the value
  for (mainItts = 0; mainItts < numIts; mainItts++)
  {
    counter++;
    //printf("Thread 1: %d\n", counter);
  }
  pthread_join(tid,NULL);
  //run(1);
  printf("after %d iterations, value of counter is...\n%d\n\n", numIts, counter);
  return 0;
}

void *run()
{
  //counter for iteration variables
  threadItts = 0;
  for (threadItts = 0; threadItts < numIts; threadItts++)
  {
    counter--;
    //printf("Thread 2: %d\n", counter);
  }
  //printf("threading works\n");
  pthread_exit(0);
}
