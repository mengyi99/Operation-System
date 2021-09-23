#include "cond.h"
#include "lock.h"
#include "stdio.h"
#include "syscall.h"
#include "test3.h"
#include "time.h"

mutex_lock_t mutex;
condition_t condition;
static int num_staff = 0;

void producer_task(void)
{
    int i;
    int print_location = 1;
    int production = 3;
    int sum_production = 0;

    for (i = 0; i < 50; i++)
    {
        do_mutex_lock_acquire(&mutex);

        num_staff += production;
        sum_production += production;

        do_mutex_lock_release(&mutex);

        vt100_move_cursor(0, print_location);
        printk("> [TASK] Total produced %d products.", sum_production);
        do_scheduler();
        do_condition_broadcast(&condition);

        // sys_sleep(1);
    }

    do_exit();
}

void consumer_task1(void)
{
    int print_location = 2;
    int consumption = 1;
    int sum_consumption = 0;

    while (1)
    {
        do_mutex_lock_acquire(&mutex);

        while (num_staff == 0)
        {
            do_condition_wait(&mutex, &condition);
        }

        num_staff -= consumption;
        sum_consumption += consumption;

        vt100_move_cursor(0, print_location);
        printk("> [TASK] Total consumed %d products.", sum_consumption);

        do_mutex_lock_release(&mutex);
        do_scheduler();
        //   sys_sleep(1);
    }
}

void consumer_task2(void)
{
    int print_location = 3;
    int consumption = 1;
    int sum_consumption = 0;

    while (1)
    {
        do_mutex_lock_acquire(&mutex);

        while (num_staff == 0)
        {
            do_condition_wait(&mutex, &condition);
        }

        num_staff -= consumption;
        sum_consumption += consumption;

        vt100_move_cursor(0, print_location);
        printk("> [TASK] Total consumed %d products.", sum_consumption);

        do_mutex_lock_release(&mutex);
                do_scheduler();
        //  sys_sleep(1);
    }
}