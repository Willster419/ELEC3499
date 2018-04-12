/*
 * Willard Wider
 * 02/06/18
 * lab3.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/mman.h>

void calcFib(int numIterations);

int first = 1;
int last = 0;
int i;
int shm_fd;
int *ptr;

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
  //allocate the required amount of memeory
  //calcedFibs = calloc(numSequences, sizeof(int));
  //allocate the required ammount of memeory the right way
  //create the file descriptor
  shm_fd = shm_open("fibs", O_CREAT|O_RDWR,0666);
  //...and set the size
  ftruncate(shm_fd, numSequences*sizeof(int));
  //..and map it
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
    ptr = mmap(0, numSequences*sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd,0);
    printf("child: starting fib calc...\n");
    calcFib(numSequences);
    printf("child: finished fib calc\n");
  }
  else
  {
    //parent process
    ptr = mmap(0, numSequences*sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd,0);
    wait(NULL);
    printf("parent: child complete\n");
    for(i = 0; i < numSequences+1; i++)
    {
      printf("%d", ptr[i]);
      if(i != numSequences)
      {
        printf(", ");
      }
    }
    printf("\ndone\n");
  }
}

//create the fib numbahs
void calcFib(int numIterations)
{
  //printf("1, 1, ");
  //printf("DEBUG: before setting, value is %d\n",ptr[0]);
  ptr[0] = 1;
  //printf("DEBUG: after setting, value is %d\n",ptr[0]);
  for(i = 1; i < numIterations+1; i++)
  {
    int total = first + last;
    last = first;
    first = total;
    ptr[i] = total;
    printf("DEBUG: index %d is now set to %d\n", i, total);
    //printf("%d, ",total);
  }
  //DEBUG
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
  //END DEBUG
  //printf("\n");
  exit(0);
}
