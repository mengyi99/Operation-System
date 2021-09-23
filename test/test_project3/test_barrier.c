#include "barrier.h"
#include "stdio.h"
#include "syscall.h"
#include "test3.h"
#include "time.h"

barrier_t barrier;
static int is_init = 0;
static int num_task = 3;

void barrier_task1(void)
{
    int i;
    int print_location = 1;

    if (!is_init)
    {
        is_init = 1;
        do_barrier_init(&barrier, num_task);
    }

    for (i = 0;; i++)
    {
        vt100_move_cursor(0, print_location);
        printk("> [TASK] Ready to enter the barrier.(%d)", i);

        do_barrier_wait(&barrier);

        vt100_move_cursor(0, print_location);
        printk("> [TASK] Exited barrier (%d).           ", i);
        do_scheduler();
//        sys_sleep(1);
    }
}

void barrier_task2(void)
{
    int i;
    int print_location = 2;

    if (!is_init)
    {
        is_init = 1;
        do_barrier_init(&barrier, num_task);
    }

    for (i = 0;; i++)
    {
        vt100_move_cursor(0, print_location);
        printk("> [TASK] Ready to enter the barrier.(%d)", i);

        do_barrier_wait(&barrier);

        vt100_move_cursor(0, print_location);
        printk("> [TASK] Exited barrier (%d).           ", i);
        do_scheduler();
  //      sys_sleep(1);
    }
}

void barrier_task3(void)
{
    int i;
    int print_location = 3;

    if (!is_init)
    {
        is_init = 1;
        do_barrier_init(&barrier, num_task);
    }

    for (i = 0;; i++)
    {
        vt100_move_cursor(0, print_location);
        printk("> [TASK] Ready to enter the barrier.(%d)", i);

        do_barrier_wait(&barrier);

        vt100_move_cursor(0, print_location);
        printk("> [TASK] Exited barrier (%d).           ", i);
        do_scheduler();
 //       sys_sleep(1);
    }
}