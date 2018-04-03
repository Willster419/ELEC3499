/*
 * Willard Wider
 * 1/22/18
 * lab1.c
 */
//we need these
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int main(void)
{
  printf("Hello World!\n");
  //print a random int value
  int randum = 0;
  //between 0 and 99
  //formula: % (max + 1 - min) + min
  srand(time(NULL));
  randum = rand() % (100);
  printf("Random value is %d", randum);
  //print a fresh line
  printf("\n");
}
