////////////////////////////////////////////////
// Willard Wider
// Date: 1/29/2018
// ELEC 3499
// simple.c
// Creating a kernel module with a linked list
////////////////////////////////////////////////

#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/list.h>
#include <linux/types.h>
#include <linux/slab.h>

struct birthday {
  int day;
  int month;
  int year;
  struct list_head list;
};

//the person pointer
struct birthday *p1, *p2, *p3, *p4, *p5;

//init the list
static LIST_HEAD(birthday_list);

/* This function is called when the module is loaded. */
int simple_init(void)
{
      printk(KERN_INFO "Loading Module\n");
      //make people
      //p1
      p1 = kmalloc(sizeof(*p1), GFP_KERNEL);
      p1->day = 2;
      p1->month = 8;
      p1->year = 1995;
      //p2
      p2 = kmalloc(sizeof(*p2), GFP_KERNEL);
      p2->day = 3;
      p2->month = 9;
      p2->year = 1996;
      //p3
      p3 = kmalloc(sizeof(*p3), GFP_KERNEL);
      p3->day = 4;
      p3->month = 10;
      p3->year = 1997;
      //p4
      p4 = kmalloc(sizeof(*p4), GFP_KERNEL);
      p4->day = 5;
      p4->month = 11;
      p4->year = 1998;
      //p5
      p5 = kmalloc(sizeof(*p5), GFP_KERNEL);
      p5->day = 6;
      p5->month = 12;
      p5->year = 1999;
      //init to the type of struct birthday
      INIT_LIST_HEAD(&p1->list);
      printk(KERN_INFO "Horray people are made");
      //adding them to the end of the list
      list_add_tail(&p1->list, &birthday_list);
      list_add_tail(&p2->list, &birthday_list);
      list_add_tail(&p3->list, &birthday_list);
      list_add_tail(&p4->list, &birthday_list);
      list_add_tail(&p5->list, &birthday_list);
      printk(KERN_INFO "Done making people and adding them to the list");
      //now let's print them out
      int i = 0;
      list_for_each_entry(p1, &birthday_list, list)
      {
        //on each iteration person points to the next struct of type birthday
        printk("person %d\nyear: %d, month: %d, day: %d\n", i, p1->year, p1->month, p1->day);
        i = i+1;
      }
      //and reset it in case we want to use it later
      i = 0;
      return 0;
}

/* This function is called when the module is removed. */
void simple_exit(void) {
  printk(KERN_INFO "Removing Module\n");
  //remove the 5 people
  list_for_each_entry_safe(p1,p2,&birthday_list,list)
  {
    list_del(&p1->list);
    kfree(p1);
  }
  printk(KERN_INFO "Horray they are removed\n");
}

/* Macros for registering module entry and exit points. */
module_init( simple_init );
module_exit( simple_exit );

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Simple Module");
MODULE_AUTHOR("SGG");
