/*
 * Willard Wider
 * 03/02/18
 * lab6.c
 * lab on threading and queues
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

#define NUM_CHARS 4
#define DEBUG 0

//variables for the petersons method
//whose turn it is to enter the crtiical section
int turn;
//if thread n is ready for entry into the critical process
int ready[2];

//variables for the queue
//how full is the queue
int counter;
//keeping track of updating the producer
int in;
//keeping track of updating the consumer
int out;
//the actual queue
char queue[NUM_CHARS];

//variables for keeping track of how much produced and consumed
int total_sent;
int total_recieved;
int total_string_size;

//the method to incriment and decriment
void *run();

//thread and it's attributes
  pthread_t tid;

int main(int argc,char* argv[])
{
  //init main stuff
  counter = 0;
  in = 0;
  out = 0;
  total_sent = 0;
  total_recieved = 0;
  total_string_size = 0;
  //make the "string"
  char text[] = "Hello world, this is COM3499!";
  //computes the length of the string up to, but not including the terminating null character
  total_string_size = strlen(text)+1;
  if (DEBUG)
    printf("DEBUG: total_string_size is %d\n",total_string_size);
  
  //create the thread and set it to run the run method
  if(pthread_create(&tid,NULL,run,NULL))
  {
    printf("failed to create thread\n");
    return -1;
  }
  //printf("parent: child process created, sending values...\n");
  while(1)
  {
    //if the buffer is full do nothing
    while(counter == NUM_CHARS) ;
    //buffer has at least one spot available, set it
    ready[0] = 1;
    turn = 1;
    while((ready[1] == 1) && (turn==1)) ;
    //printf("DEBUG: parent entered\n");
    queue[in] = text[total_sent];
    in = (in+1) % NUM_CHARS;
    counter++;
    ready[0] = 0;
    total_sent++;
    if (DEBUG)
      printf("DEBUG: parent sending %c, counter =%d, in=%d,total_sent=%d\n",text[total_sent-1],counter,in,total_sent);
    if(total_sent >= total_string_size)
      break;
  }
  //wait for it
  pthread_join(tid,NULL);
  //aaand that's it
  //printf("parent: done\n");
  return 0;
}

void *run()
{
  //printf("child: listinging for the string...\n");
  //DEBUG:
  char recieved[total_string_size];
  while(1)
  {
    //if 0, nothing to recieve...
    while(counter == 0) ;
    ready[1] = 1;
    turn = 0;
    while((ready[0] == 1) && (turn==0)) ;
    //printf("DEBUG: child entered\n");
    recieved[total_recieved] = queue[out];
    printf("%c",queue[out]);
    //sleep(1);
    out = (out+1) % NUM_CHARS;
    counter--;
    ready[1] = 0;
    total_recieved++;
    if (DEBUG)
      printf("DEBUG: child recieved value: %c, counter=%d, out=%d,total_revieced=%d\n",recieved[total_recieved-1],counter,out,total_recieved);
    if(total_recieved >= total_string_size)
      break;
  }
  printf("\n");
  //printf("\nchild: done, result is\n%s\n", recieved);
  pthread_exit(0);
  //return 0;
}
