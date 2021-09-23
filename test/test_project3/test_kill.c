#include "lock.h"
#include "sched.h"
#include "stdio.h"
#include "syscall.h"
#include "test3.h"
#include "time.h"

static char blank[] = {"                                                "};

mutex_lock_t lock1;
mutex_lock_t lock2;

// static struct task_info task1 = {"task1", (uint32_t)&wait_kill_task1, USER_PROCESS};
// static struct task_info task2 = {"task2", (uint32_t)&wait_kill_task2, USER_PROCESS};

// void killer_task()
// {
//     int i, print_location = 0;
//
//     sys_move_cursor(0, print_location);
//     printf("> [TASK] There are still 5 seconds to kill the target mission.");
//
//     sys_sleep(5);
//
//     sys_kill(2);
//
//    sys_move_cursor(0, print_location);
//    printf("> [TASK] The mission is completed and the target process has been killed.", i);
//
//   sys_exit();
// }

// pid = 2
void ready_to_exit_task()
{
    int i = 0, print_location = 1;

    do_mutex_lock_acquire(&lock1);
    do_mutex_lock_acquire(&lock2);

    // sys_spawn(&task1);
    // sys_spawn(&task2);

    for (i = 0; i < 500; i++)
    {
        vt100_move_cursor(0, print_location);
        uint32_t core_id = get_cpu_id();
        printk("> [TASK] I am task with pid %d, I have acquired two mutex lock. (%d)", current_running->pid + 1, i++);//current_running[core_id]->pid, i++
        do_scheduler();
    }
    do_exit(); // test exit
}

// pid = 3
void wait_lock_task()
{
    int i, print_location = 2;

    vt100_move_cursor(0, print_location);
    printk("> [TASK] I want to acquire a mute lock from task(pid=2).");

    do_mutex_lock_acquire(&lock1);

    vt100_move_cursor(0, print_location);
    printk("> [TASK] I have acquired a mutex lock from task(pid=2).");
    do_scheduler();
    do_exit(); // test exit
}

// pid = 4
void wait_exit_task()
{
    int i, print_location = 3;

    vt100_move_cursor(0, print_location);
    printk("> [TASK] I want to wait task (pid=2) to exit.");

    do_waitpid(2); //test waitpid

    vt100_move_cursor(0, print_location);
    printk("> [TASK] Task (pid=2) has exited.                ");
    do_scheduler();
    do_exit(); // test exit
}