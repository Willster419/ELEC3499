/*
 * Willard Wider
 * 04/02/18
 * lab9.c
 * lab on 
 */

//usefull headers to include
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <stdbool.h>
#include <sys/types.h>
#include <time.h>

//usefull defines
#define BILLION 1E9
#define DEBUG 1
#define RANDOM_PAGE_TRACE_LENGTH 10
#define MIN_PAGE_NUMBER 0
#define MAX_PAGE_NUMBER 31
#define MODE 0//0=LRU,1=OPT

//globals
int num_faults = 0;
int num_frames_alocated = 0;
int *page_trace;
const char delim[2] = ",";
int page_trace_length = 0;

//method sigs
void LRU(int address);
void optimal(int address);
void add_page(int address);
bool is_full();
void inc_staleness();
void evict_and_replace(int address);
void evict_and_replace_old(int address);
void print_page_table();

struct Page
{
  int address;
  bool is_head;
  bool is_tail;
  int staleness;
  bool is_empty;
  struct Page *next;
  struct Page *prev;
};

struct Page *head;
struct Page *tail;

int main(int argc,char* argv[])
{
  //first arguement: num of frames allocated
  if(argv[1] != NULL)
    num_frames_alocated = atoi(argv[1]);
  if(num_frames_alocated < 4 || num_frames_alocated > 24)
  {
    printf("ERROR: invalid num_frames_alocated: %d\naccepted number must be between 4 and 24\n",num_frames_alocated);
    return -1;
  }
  //second arguement (optional): page trace seperated by commas
  if(argv[2] != NULL)
  {
    if(DEBUG)
      printf("DEBUG: parsing command line arguement\n");
    int counter = 0;
    //parse the page trace to an ints
    char *page_trace_char;
    page_trace_char = strtok(argv[2], delim);
    while (page_trace_char != NULL)
    {
      page_trace[counter++] = atoi(page_trace_char);
      if(DEBUG)
        printf("DEBUG: int parsed: %d, index %d\n", page_trace[counter-1], counter);
      page_trace_char = strtok(NULL,delim);
    }
    page_trace_length = counter;
  }
  else
  {
    if(DEBUG)
      printf("DEBUG: making random page trace\n");
    //randomly generate the page trace
    page_trace = malloc(sizeof(int)*RANDOM_PAGE_TRACE_LENGTH);
    srand(time(NULL));
    for(int i = 0; i < RANDOM_PAGE_TRACE_LENGTH; i++)
    {
      //between 0 and 31
      //formula: % (max + 1 - min) + min
      page_trace[i] = rand() % (31 + 1 - 0) + 0;
      if(DEBUG)
        printf("DEBUG: Random page trace value is %d for sequence %d of 10\n", page_trace[i], i);
    }
    page_trace_length = RANDOM_PAGE_TRACE_LENGTH;
  }
  //create the page table stack
  //creates a static list for num_frames_alocated
  struct Page *last;
  for(int i = 0; i < num_frames_alocated+1; i++)
  {
    struct Page *temp;
    temp = malloc(sizeof(struct Page));
    if(i==0)
    {
      head = temp;
      head->is_head = true;
      head->is_tail = false;
      head->address = -1;
      head->staleness = 0;
      head->is_empty = true;
      head->next = NULL;
      head->prev = NULL;
      last = head;
    }
    else if (i+1==num_frames_alocated)
    {
      temp->is_head = false;
      temp->is_tail = true;
      temp->is_empty = true;
      temp->address = -1;
      temp->staleness = 0;
      last->next = temp;
      temp->prev = last;
      temp->next = NULL;
    }
    else
    {
      temp->is_head = false;
      temp->is_tail = false;
      temp->address = -1;
      temp->staleness = 0;
      temp->is_empty = true;
      last->next = temp;
      temp->prev = last;
      temp->next = NULL;
    }
    last = temp;
  }
  //print current page table?
  if(DEBUG)
  {
    printf("DEBUG: print page table\n");
    print_page_table();
  }
  //run the page simulation
  if(DEBUG)
      printf("DEBUG: simulation starts, MODE=%d, (%s)\n",MODE, MODE==0? "LRU":"OPT");
  for(int i = 0; i < page_trace_length; i++)
  {
    switch(MODE)
    {
      case 0:
        LRU(page_trace[i]);
      break;
      case 1:
        optimal(page_trace[i]);
      break;
      default:
        printf("ERROR: Invalid case from MODE: %d\n",MODE);
        return -1;
      break;
    }
    printf("after round %d, page table is as follows:\n",i+1);
    print_page_table();
  }
  printf("Simluation ends, num_faults=%d\n",num_faults);
  return 0;
}

void LRU(int address)
{
  bool page_found = false;
  //check for the page and reset staleness
  struct Page *ref;
  ref = head;
  int counter = 0;
  if(DEBUG)
    printf("DEBUG: checking if address %d is in page table...\n",address);
  while(ref->next != NULL)
  {
    if((ref->address == address) && (!ref->is_empty))
    {
      printf("horray, address %d found in table!\n",address);
      //set to -1 cause it will be set to 0 later
      ref->staleness = -1;
      page_found = true;
      break;
    }
    ref = ref->next;
  }
  //ONLY DO THIS IF FAULT!!!
  if(!page_found)
  {
    printf("page fault found, requested address=%d\n",address);
    if(is_full())
    {
      printf("page table is full, evict/replace and inc num_faults\n");
      evict_and_replace(address);
      num_faults++;
    }
    else
    {
      printf("page table not full, \"add\" to buttom of stack\n");
      /*old method
      tail->is_tail = false;
      struct Page *temp;
      temp = malloc(sizeof(struct Page));
      temp->is_head = false;
      temp->is_tail = true;
      temp->address = address;
      temp->staleness = 0;
      temp->next = NULL;
      temp->prev = tail;
      tail->next = temp;
      tail = temp;
      */
      //new method:
      //don't make a new one, just set the newest one with values
      add_page(address);
    }
  }
  inc_staleness();
}

void optimal(int address)
{
  
}

bool is_full()
{
  int countah = 0;
  struct Page *ref;
  ref = head;
  while(ref->next != NULL)
  {
    if(!ref->is_empty)
      countah++;
    ref = ref->next;
  }
  if(countah < num_frames_alocated)
  {
    if(DEBUG)
      printf("DEBUG: is_full returns false\n");
    return false;
  }
  else
  {
    if(DEBUG)
      printf("DEBUG: is_full returns true\n");
    return true;
  }
}

void inc_staleness()
{
  struct Page *ref;
  ref = head;
  int counter = 0;
  while(ref->next != NULL)
  {
    if(!ref->is_empty)
      ref->staleness++;
    if(DEBUG)
      printf("DEBUG: increasing staleness of page %d with now staleness of %d\n",counter++,ref->staleness);
    ref = ref->next;
  }
}

void print_page_table()
{
  struct Page *ref;
  ref = head;
  int counter = 0;
  while(ref->next != NULL)
  {
    printf("Page entry %d: address=%d, staleness=%d, is_head=%d, is_tail=%d, is_empty=%d\n",
    counter++,ref->address,ref->staleness,ref->is_head,ref->is_tail,ref->is_empty);
    ref = ref->next;
  }
}

void evict_and_replace_old(int address)
{
  //find the most stale page
  struct Page *ref;
  struct Page *most_stale;
  int max_staleness = 0;
  ref = head;
  int counter = 0;
  while(ref->next != NULL)
  {
    if(ref->staleness >= max_staleness)
    {
      max_staleness = ref->staleness;
      most_stale = ref;
    }
    ref = ref->next;
  }
  //we now have the most stale version
  //make a new page
  struct Page *new;
  new = malloc(sizeof(struct Page));
  new->address = address;
  new->staleness = -1;
  //TODO: checks for if next to head or tail
  new->next = ref->next;
  ref->prev->next = new;
  new->prev = ref;
  ref->next = NULL;
  ref->prev = NULL;
}

void evict_and_replace(int address)
{
  //find the most stale page
  struct Page *ref;
  struct Page *most_stale;
  int max_staleness = 0;
  ref = head;
  int counter = 0;
  while(ref->next != NULL)
  {
    if(ref->staleness >= max_staleness)
    {
      max_staleness = ref->staleness;
      most_stale = ref;
    }
    ref = ref->next;
  }
  //we now have the most stale version
  //evict the old address and add the new address
  //still has data, don't touch refrences, etc
  if(DEBUG)
    printf("DEBUG: evicted page address=%d, new page address = %d\n",
      most_stale->address,address);
  most_stale->address = address;
  most_stale->staleness = -1;
}

void add_page(int address)
{
  struct Page *ref;
  int max_staleness = 0;
  ref = head;
  int counter = 0;
  while(ref->next != NULL)
  {
    if(ref->is_empty)
      break;
    ref = ref->next;
    counter++;
  }
  //we now have the latest one
  ref->address = address;
  ref->is_empty = false;
  //because it will get inc'd later
  ref->staleness = -1;
  if(DEBUG)
    printf("DEBUG: \"adding\" page of address %d to page location %d\n",address,counter);
}