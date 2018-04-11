/*
 * Willard Wider
 * 04/11/18
 * lab9.c
 * lab on page tables
 * v2 done correctly
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
//#define DEBUG 1
#define PAGE_TRACE_LENGTH 50//set to 500 later
#define MIN_PAGE_ADDRESS 0
#define MAX_PAGE_ADDRESS 31
#define MIN_FRAMES_ALLOWED 4
#define MAX_FRAMES_ALLOWED 24

//globals
int num_faults = 0;
int num_frames_alocated = 0;
int *page_trace;
int DEBUG = 0;

struct Page
{
  int address;
  bool is_head;
  bool is_tail;
  int staleness;
  bool is_empty;
  bool used_in_future;
  int future_staleness;
  struct Page *next;
  struct Page *prev;
};

//method sigs
void LRU(int address, int num_page_tables);
void optimal(int address, int current_page_trace_index, int num_page_tables);
void add_page(int address);
bool is_full(int num_page_tables);
void inc_staleness();
void evict_and_replace_LRU(int address);
void evict_and_replace_old(int address);
void evict_and_replace_opt(int address, int current_page_trace_index);
void print_page_table();
void add_page_old();
bool find_page(int address);
void create_page_table(int num_frames_alocated);
void free_page_table(struct Page *p);
void reset_page_table();

struct Page *head;
struct Page *tail;

int main(int argc,char* argv[])
{
  if(argv[1] != NULL)
    DEBUG = atoi(argv[1]);
  printf("making random page trace\n");
  //randomly generate the page trace
  page_trace = malloc(sizeof(int)*PAGE_TRACE_LENGTH);
  srand(time(NULL));
  for(int i = 0; i < PAGE_TRACE_LENGTH; i++)
  {
    //between 0 and 31
    //formula: % (max + 1 - min) + min
    page_trace[i] = rand() % (31 + 1 - 0) + 0;
    if(DEBUG)
      printf("DEBUG: Random page trace value is %d for sequence %d of %d\n",
        page_trace[i], i, PAGE_TRACE_LENGTH);
  }
  create_page_table(MIN_FRAMES_ALLOWED);
  if(DEBUG>1)
  {
    //print current page table
    printf("at start, page table is:\n");
    print_page_table();
  }
  //run the page simulations
  //LRU first, then optimal
  printf("Starting LRU (1 of 2)...\n");
  for(int i = MIN_FRAMES_ALLOWED; i <= MAX_FRAMES_ALLOWED; i++)
  {
    for(int j = 0; j < PAGE_TRACE_LENGTH; j++)
    {
      LRU(page_trace[j],i);
      if(DEBUG>2)
        print_page_table();
    }
    printf("Simluation (LRU) ends with table length=%d, num_faults=%d\n",i,num_faults);
    if(DEBUG)
      printf("DEBUG: reseting simulation via free_page_table...\n");
    free_page_table(head);
    if(DEBUG)
      printf("DEBUG: reseting simulation via create_page_table...\n");
    create_page_table(i);
    //reset_page_table();
    num_faults = 0;
  }

  printf("Starting OPT (2 of 2)...\n");
  num_faults = 0;
  for(int i = MIN_FRAMES_ALLOWED; i <= MAX_FRAMES_ALLOWED; i++)
  {
    for(int j = 0; j < PAGE_TRACE_LENGTH; j++)
    {
      optimal(page_trace[j],j,i);
      if(DEBUG>2)
        print_page_table();
    }
    printf("Simluation (OPT) ends with table length=%d, num_faults=%d\n",i,num_faults);
    if(DEBUG)
      printf("DEBUG: reseting simulation via free_page_table...\n");
    free_page_table(head);
    if(DEBUG)
      printf("DEBUG: reseting simulation via create_page_table...\n");
    create_page_table(i);
    num_faults = 0;
  }
  free_page_table(head);
  printf("Done\n");
  return 0;
}

void LRU(int address, int num_page_tables)
{
  bool page_found = find_page(address);
  if(!page_found)
  {
    if(DEBUG)
      printf("DEBUG: page fault found, requested address=%d\n",address);
    if(is_full(num_page_tables))
    {
      if(DEBUG)
        printf("DEBUG: page table is full, evict/replace and inc num_faults\n");
      evict_and_replace_LRU(address);
    }
    else
    {
      if(DEBUG)
        printf("DEBUG: page table not full, \"add\" to buttom of stack and inc num_faults\n");
      add_page(address);
    }
    num_faults++;
  }
  inc_staleness();
}

void optimal(int address, int current_page_trace_index, int num_page_tables)
{
  bool page_found = find_page(address);
  if(!page_found)
  {
    if(DEBUG)
      printf("page fault found, requested address=%d\n",address);
    if(is_full(num_page_tables))
    {
      if(DEBUG)
        printf("page table is full, evict/replace and inc num_faults\n");
      evict_and_replace_opt(address,current_page_trace_index);
    }
    else
    {
      if(DEBUG)
        printf("page table not full, \"add\" to buttom of stack and inc num_faults\n");
      add_page(address);
    }
    num_faults++;
  }
  //staleness does not matter i this one
  //inc_staleness();
}

bool is_full(int num_page_tables)
{
  //gets the exact number of page tables empty
  int num_empty_pages = 0;
  struct Page *ref;
  ref = head;
  while(ref != NULL)
  {
    if(ref->is_empty)
      num_empty_pages++;
    ref = ref->next;
  }
  if(DEBUG)
    printf("DEBUG: number of empty pages=%d\n",num_empty_pages);
  if(num_empty_pages==0)
  {
    if(DEBUG)
      printf("DEBUG: is_full returns true\n");
    return true;
  }
  else
  {
    if(DEBUG)
      printf("DEBUG: is_full returns false\n");
    return false;
  }
}

void inc_staleness()
{
  struct Page *ref;
  ref = head;
  int counter = 0;
  while(ref != NULL)
  {
    if(!ref->is_empty)
      ref->staleness++;
    if(DEBUG>1)
      printf("DEBUG: increasing staleness of page %d with now staleness of %d\n",counter++,ref->staleness);
    ref = ref->next;
  }
}

void print_page_table()
{
  struct Page *ref;
  ref = head;
  int counter = 0;
  while(ref != NULL)
  {
    printf("Page entry %d: address=%d, is_head=%d, is_tail=%d, is_empty=%d, staleness=%d,\n",
      counter++,ref->address,ref->is_head,ref->is_tail,ref->is_empty,ref->staleness);
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
  while(ref != NULL)
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

void evict_and_replace_LRU(int address)
{
  //find the most stale page
  struct Page *ref;
  struct Page *most_stale;
  int max_staleness = 0;
  ref = head;
  int counter = 0;
  while(ref != NULL)
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
    printf("DEBUG: evicted page address=%d, new page address = %d\n",most_stale->address,address);
  most_stale->address = address;
  most_stale->staleness = -1;
}

void evict_and_replace_opt(int address, int current_page_trace_index)
{
  //reset used_in_future and future_staleness
  struct Page *ref;
  ref = head;
  if(DEBUG)
    printf("DEBUG: reset used_in_future and future_staleness values\n");
  while(ref != NULL)
  {
    ref->used_in_future = false;
    ref->future_staleness = 0;
    ref = ref->next;
  }
  //also know that getting here means that the page table is full
  //start counting from the current place in trace (not including current address)
  //count out (if not used in ifture) the future staleness
  int cycle_counter = 0;
  for(int i = current_page_trace_index; i < PAGE_TRACE_LENGTH; i++)
  {
    if(DEBUG>1)
      printf("DEBUG: checking for page trace index %d, value=%d\n",i,page_trace[i]);
    struct Page *temp;
    temp = head;
    while(temp != NULL)
    {
      if(!temp->used_in_future)
      {
        temp->future_staleness++;
      }
      if(page_trace[i]==temp->address && !temp->used_in_future)
      {
        temp->used_in_future = true;
        if(DEBUG>1)
          printf("DEBUG: page at address %d used %d cycles in future, bool set\n",temp->address,cycle_counter+1);
      }
      temp = temp->next;
    }
    cycle_counter++;
  }
  //if after done, we find a not_used_in_future, evict
  ref = head;
  //re-use cycle counter
  cycle_counter = 0;
  bool found_page_for_removal = false;
  struct Page *max_future_stale_page;
  while(ref != NULL)
  {
    if(!ref->used_in_future)
    {
      if(DEBUG>1)
        printf("DEBUG: found that page %d is never used in future, selected for removal (address=%d)\n",cycle_counter,ref->address);
      max_future_stale_page = ref;
      found_page_for_removal = true;
      break;
    }
    cycle_counter++;
    ref = ref->next;
  }
  //else, find the maximum future staleness and evict it
  if(!found_page_for_removal)
  {
    //re-use cycle counter again
    cycle_counter = 0;
    int max_future_stale = -1;
    ref=head;
    while(ref != NULL)
    {
      if(ref->future_staleness > max_future_stale)
      {
        max_future_stale_page = ref;
        max_future_stale=ref->future_staleness;
      }
      cycle_counter++;
      ref = ref->next;
    }
    if(DEBUG)
      printf("DEBUG: found page that is most future stale (value=%d), selected for removal (address=%d)\n",
    max_future_stale_page->future_staleness,max_future_stale_page->address);
  }
  //we now have the most stale version
  //evict the old address and add the new address
  //still has data, don't touch refrences, etc
  if(DEBUG)
    printf("DEBUG: evicted page address=%d, new page address = %d\n", max_future_stale_page->address,address);
  max_future_stale_page->address = address;
}

void add_page(int address)
{
  struct Page *ref;
  int max_staleness = 0;
  ref = head;
  int counter = 0;
  while(ref != NULL)
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

void add_page_old()
{
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
}

bool find_page(int address)
{
  //check for the page and reset staleness
  struct Page *ref;
  ref = head;
  int counter = 0;
  if(DEBUG)
    printf("DEBUG: checking if address %d is in page table...\n",address);
  while(ref != NULL)
  {
    if((ref->address == address) && (!ref->is_empty))
    {
      if(DEBUG)
        printf("DEBUG: address %d found in table!\n",address);
      //set to -1 cause it will be set to 0 later
      ref->staleness = -1;
      //page_found = true;
      return true;
      //break;
    }
    ref = ref->next;
  }
  return false;
}

void create_page_table(int num_frames_alocated)
{
  //create the page table stack
  //creates a static list for num_frames_alocated
  struct Page *last;
  for(int i = 0; i < num_frames_alocated; i++)
  {
    struct Page *temp;
    temp = malloc(sizeof(struct Page));
    if(i==0)//is first one
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
    else if (i+1==num_frames_alocated)//is last one
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
}

void free_page_table(struct Page *p)
{
  //free page table from memory
  if(p->next != NULL)
    free_page_table(p->next);
  free(p);
}

void reset_page_table()
{
  //reset the page values back to when it was created
  struct Page *ref;
  ref = head;
  int counter = 0;
  if(DEBUG)
    printf("DEBUG: resetting pages...\n");
  while(ref != NULL)
  {
    if(DEBUG>1)
        printf("DEBUG: resetting page %d\n",counter++);
    ref->is_empty = true;
    ref->address = -1;
    ref->staleness = 0;
    ref->used_in_future = false;
    ref->future_staleness = 0;
    ref = ref->next;
  }
}
