/*
 * Willard Wider
 * 02/13/18
 * lab4.c
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

void calcFib(int numIterations);

int first = 1;
int last = 0;
int i;//counter
//the pipe to use for piping the int values of the child process
//index 0 = reading, index 1 = writing
//child is writing, parrent is reading
int fibPipe[2];
int pipedFib;

int main(int argc,char* argv[])
{
  //the child process
  pid_t pid;
  //argv[0] is the program name
  //printf("program name is: %s\n",argv[0]);
  if(argc != 2)
  {
    printf("must supply 1 parameter of type int\n");
    return -1;
  }
  int numSequences = atoi(argv[1]);
  if (numSequences < 1)
  {
    printf("invalid number of sequences\n");
    return -1;
  }
  printf("requesting %d fib sequences...\n", numSequences);
  //create the pipe
  if(pipe(fibPipe) != 0)
  {
    printf("pipe creation failed\n");
  }
  
  //to do it in the main thread
  //calcFib(numSequences);
  printf("creating child process\n");
  pid = fork();
  if(pid < 0)
  {
    printf("failed to create child process\n");
    return -1;
  }
  if(pid == 0)
  {
    //child process
    printf("child: starting fib calc...\n");
    //close index 0 in the child so it only writes
    close(fibPipe[0]);
    calcFib(numSequences);
    //when done, close the pipe
    close(fibPipe[1]);
  }
  else
  {
    //parent process
    printf("parent: listing for child values\n");
    //close index 1 in the parrent so it only reads
    i = 0;
    pipedFib = 0;
    close(fibPipe[1]);
    while(true)
    {
      //usleep(10000);//sleep 10ms
      read(fibPipe[0],&pipedFib,sizeof(int));
      printf("parent: value returned, %d\n",pipedFib);
      //printf("DEBUG: value address %d\n", &pipedFib);
      i+=1;
      //break out when it is done
      if(i == numSequences)
      {
        printf("parent: complete\n");
        //close the pipe
        close(fibPipe[0]);
        exit(0);
      }
    }
  }
}

//create the fib numbahs
void calcFib(int numIterations)
{
  int randum = 0;
  for(i = 1; i < numIterations+1; i++)
  {
    int total = first + last;
    last = first;
    first = total;
    //printf("DEBUG: total= %d, total address = %d\n", total, &total);
    //formula: % (max + 1 - min) + min
    //10000 = 10ms = 0.01 sec
    //100000 = 100ms = 0.1 sec
    //1000000 = 1000ms = 1 sec
    srand(time(NULL));
    randum = rand() % (1000000 + 1 - 0) + 0;
    usleep(randum);//sleep 10ms
    //pipe the value
    write(fibPipe[1], &total, sizeof(int));
    //sleep(1);
    //printf("%d, ",total);
  }
  //DEBUG
  /*
  printf("DEBUG: child printing values...\n");
  for(i = 0; i < numIterations+1; i++)
  {
    printf("%d", ptr[i]);
    if(i != numIterations)
    {
      printf(", ");
    }
  }
  printf("\n");
  */
  //END DEBUG
  //printf("\n");
}
