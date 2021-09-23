#include "sched.h"
#include "stdio.h"
#include "syscall.h"
#include "test3.h"
#include "time.h"

static char blank[] = {"                                                "};

void waiting_task1(void)
{
    int print_location = 1;

    vt100_move_cursor(0, print_location);
    printk("> [TASK] This is a waiting task (pid=%d).\n", print_location);

    do_waitpid(3);

    vt100_move_cursor(0, print_location);
    printk("> [TASK] I am waiting task and already exited successfully.\n");
    do_scheduler();
    do_exit();
}

void waiting_task2(void)
{
    int print_location = 2;

    vt100_move_cursor(0, print_location);
    printk("> [TASK] This is a waiting task (pid=%d).\n", print_location);

    do_waitpid(3);

    vt100_move_cursor(0, print_location);
    printk("> [TASK] I am waiting task and already exited successfully.\n");
    do_scheduler();
    do_exit();
}

void waited_task(void)
{
    int i, print_location = 3;

    vt100_move_cursor(0, print_location);
    printk("> [TASK] I still have 5 seconds to quit.\n");

//    sys_sleep(5);
    do_scheduler();
    do_exit();
}