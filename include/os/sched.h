/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *  * * * * * * * * * * *
 *            Copyright (C) 2018 Institute of Computing Technology, CAS
 *               Author : Han Shukai (email : hanshukai@ict.ac.cn)
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *  * * * * * * * * * * *
 *        Process scheduling related content, such as: scheduler, process blocking,
 *                 process wakeup, process creation, process kill, etc.
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *  * * * * * * * * * * *
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *  * * * * * * * * * * */

#ifndef INCLUDE_SCHEDULER_H_
#define INCLUDE_SCHEDULER_H_

#include "type.h"
#include "queue.h"

#define NUM_MAX_TASK 32
#define CORE_NUM 2

#define  STACK_TOP 0xffffffffa0f00000
#define  STACK_SIZE 0x1000
#define  STACK_USER_TOP 0xffffffffa0f20000
#define NUM_KERNEL_STACK 20

#define PRIOR_MAX 5

#define STATE_KERNEL 0
#define STATE_USER 1

#define SEC_SLICE 7000
/* used to save register infomation */
typedef struct regs_context
{
 
    uint64_t regs[32];

    uint64_t cp0_status;
    uint64_t cp0_cause;
    uint64_t cp0_count;
    uint64_t cp0_compare;
    uint64_t cp0_epc;
    uint64_t hi, lo;
    uint64_t pc;
} regs_context_t; 

typedef enum
{
    TASK_BLOCKED, // 0
    TASK_RUNNING, // 1
    TASK_READY,   // 2
    TASK_EXITED,  // 3
} task_status_t;

typedef enum
{
    KERNEL_PROCESS, // 0
    KERNEL_THREAD,  // 1
    USER_PROCESS,   // 2
    USER_THREAD,    // 3
} task_type_t;

/* Process Control Block */
typedef struct pcb
{
    /* register context */
    regs_context_t  kernel_context,
                    user_context;

    uint64_t  kernel_stack_top,
              user_stack_top;
    
    //task state:kernel or user
    int state;
    
    /* previous, next pointer */
    void *prev,
         *next;

    /* task in which queue */

    /*
     * What tasks are blocked by me, the tasks in this
     * queue need to be unblocked when I do_exit().
     */

    /* holding lock */
    queue_t lock_queue;
    //wait
    queue_t wait_queue;
    /* block related */
    void *block_me;
    /* priority */
    uint32_t prior;

    // task infomation:

    // name
    char name[32];
    /* process id */
    pid_t pid;

    /* task type: kernel/user thread/process */
    task_type_t type;

    /* task status: BLOCK | READY | RUNNING | EXIT */
    task_status_t status;

    /* cursor position */
    int cursor_x,
        cursor_y;
} pcb_t;

/* task information, used to init PCB */
typedef struct task_info
{
    char name[32];
    uint64_t entry_point;
    task_type_t type;
    uint32_t prior;
} task_info_t;

/* ready queue to run */
extern queue_t ready_queue;

/* block queue to wait */
extern queue_t block_queue;

/* current running task PCB */
extern pcb_t *current_running;
extern pid_t process_id;

extern pcb_t pcb[NUM_MAX_TASK];
extern uint32_t initial_cp0_status;

void do_scheduler(void);

int do_spawn(task_info_t *);
void do_exit(void);
void do_sleep(uint32_t);

int do_kill(pid_t pid);
int do_waitpid(pid_t pid);

void do_block(queue_t *);
void do_unblock_one(queue_t *);
void do_unblock_all(queue_t *);

void init_stack();
void set_pcb(pid_t, pcb_t *, task_info_t *);

void do_process_show();
pid_t do_getpid();
uint64_t get_cpu_id();

#endif
