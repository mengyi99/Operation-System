#include "sem.h"
#include "stdio.h"
#include "syscall.h"
#include "test3.h"
#include "time.h"

semaphore_t semaphore;
static int global_count = 0;

void semaphore_add_task1(void)
{
    int i;
    int print_location = 1;
    // int sum_up = 0;

    do_semaphore_init(&semaphore, 1);

    for (i = 0; i < 10; i++)
    {
        do_semaphore_down(&semaphore); // semaphore.value--
        // semaphore = 0
        global_count++;
        do_semaphore_up(&semaphore);

        vt100_move_cursor(0, print_location);
        printk("> [TASK] current global value %d. (%d)", global_count, i + 1);
        do_scheduler();
 //       sys_sleep(1);
    }

    do_exit();
}

void semaphore_add_task2(void)
{
    int i;
    int print_location = 2;
    // int sum_down = 0;

    for (i = 0; i < 20; i++)
    {
        do_semaphore_down(&semaphore); // semaphore.value--
        // semaphore = 0
        global_count++;
        do_semaphore_up(&semaphore);

        vt100_move_cursor(0, print_location);
        printk("> [TASK] current global value %d. (%d)", global_count, i + 1);
        do_scheduler();
 //       sys_sleep(1);
    }

    do_exit();
}

void semaphore_add_task3(void)
{
    int i;
    int print_location = 3;
    // int sum_down = 0;

    for (i = 0; i < 30; i++)
    {
        do_semaphore_down(&semaphore); // semaphore.value--
        // semaphore = 0
        global_count++;
        do_semaphore_up(&semaphore);

        vt100_move_cursor(0, print_location);
        printk("> [TASK] current global value %d. (%d)", global_count, i + 1);
        do_scheduler();
 //       sys_sleep(1);
    }

    do_exit();
}