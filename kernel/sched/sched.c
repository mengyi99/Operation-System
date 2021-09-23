#include "lock.h"
#include "time.h"
#include "stdio.h"
#include "sched.h"
#include "queue.h"
#include "irq.h"
#include "screen.h"
#include "string.h"

pcb_t pcb[NUM_MAX_TASK];

/* current running task PCB */
pcb_t *current_running;

/* global process id */
pid_t process_id = 1;

uint32_t global_prior = PRIOR_MAX;

// kernel stack
static uint64_t kernel_stack[NUM_KERNEL_STACK];
static int kernel_stack_count;

// user stack
static uint64_t user_stack[NUM_KERNEL_STACK];
static int user_stack_count;
queue_t ready_queue;
queue_t block_queue;

void init_stack()
{

}
uint64_t new_kernel_stack()
{

}

uint64_t new_user_stack()
{

}

static void free_kernel_stack(uint64_t stack_addr)
{
}

static void free_user_stack(uint64_t stack_addr)
{
}

/* Process Control Block */
int alloc_stack()
{
    int i;
    for (i = 1; i < NUM_MAX_TASK; i++)
    {
        if (pcb[i].status == TASK_EXITED)
        {
            bzero(&pcb[i], sizeof(pcb_t));
            pcb[i].kernel_stack_top = STACK_TOP + i * STACK_SIZE;
            pcb[i].user_stack_top = STACK_USER_TOP + i * STACK_SIZE;
            pcb[i].status = TASK_READY;
            pcb[i].block_me = NULL;
            queue_init(&(pcb[i].lock_queue));
            queue_init(&(pcb[i].wait_queue));
            return i;
        }
    }
    return -1;
}

void set_pcb(pid_t pid, pcb_t *pcb, task_info_t *task_info)
{
    pcb->pid = pid;
    pcb->type = task_info->type;
   	pcb->prior = task_info->prior;
   	pcb->state = (task_info->type == USER_PROCESS || task_info->type == USER_THREAD) ? STATE_USER : STATE_KERNEL;
   	int i;
	  for (i = 0; i < 32; i++)
		pcb->name[i] = task_info->name[i];

	  if (pid < NUM_MAX_TASK)
		pcb->next = (pcb_t *)(&pcb + sizeof(pcb_t));
   
    // ra
    pcb->user_context.regs[31] = task_info->entry_point;
    // sp
    pcb->user_context.regs[29] = pcb->user_stack_top;
    pcb->kernel_context.regs[29] = pcb->kernel_stack_top;

}



/* ready queue to run */
queue_t ready_queue;

/* block queue to wait */
queue_t block_queue;

static void check_sleeping()
{
}

void scheduler(void)
{
  /*if(current_running->pid == 0)      //不让启动pcb回到队列
			{
      queue_push(&ready_queue,current_running);
 	    current_running->cursor_x = screen_cursor_x;
      current_running->cursor_y = screen_cursor_y;
      }
  else*/
   if(current_running->status == TASK_RUNNING )
   {
       queue_push(&ready_queue,current_running);
   }
       current_running->cursor_x = screen_cursor_x;
	     current_running->cursor_y = screen_cursor_y;
     	 current_running= (pcb_t *)queue_dequeue(&ready_queue);

  	current_running->status = TASK_RUNNING;
 	  screen_cursor_x = current_running->cursor_x;
	  screen_cursor_y = current_running->cursor_y;

}

void do_sleep(uint32_t sleep_time)
{
}

void do_exit(void)
{
	pcb_t *to_exit;
	to_exit = current_running;
//	queue_remove(&ready_queue, current_running);   /*do exit 自己在currentrunning，此句赘余*/

	// 需要exit的进程被锁住了
	while (!queue_is_empty(&(to_exit->lock_queue))) {
		mutex_lock_t *t;
		t = to_exit->lock_queue.head;
		do_unblock_all(&(t->blocked));
		queue_dequeue(&(to_exit->lock_queue));
		t->status = UNLOCKED;
	}
	//从wait queue中清空
	do_unblock_all(&(to_exit->wait_queue));

	to_exit->status = TASK_EXITED;
	do_scheduler();
}

// block current_running to '*queue'
void do_block(queue_t *queue)
{
    current_running->status = TASK_BLOCKED;
	  queue_push(queue, (void *)current_running);
    current_running->block_me =queue;
    do_scheduler();
}

// unblock head of '*queue'
void do_unblock_one(queue_t *queue)
{
    pcb_t *item = queue_dequeue(queue);
    item->status = TASK_READY;
    queue_push(&ready_queue,item);
    ((pcb_t *)item)->block_me = NULL; 
}

// unblock all items in '*queue'
void do_unblock_all(queue_t *queue)
{
    while (!queue_is_empty(queue))
    {
       do_unblock_one(queue);
    } 
}

int do_spawn(task_info_t *task)
{
//exec 1 2 3
//allocate pcb… for a task and put in ready queue
	int i;
	i = alloc_stack();
	set_pcb(i, &pcb[i], task);
	queue_push(&ready_queue, &pcb[i]);
	pcb[i].status = TASK_READY;

}

int do_kill(pid_t pid)
{
//kill 8
//relese pid lock…    readyqueue
	int i, flag;
	flag = 0;
	for (i = 0; i < NUM_MAX_TASK; i++) // 寻找需要被杀死的进程
  {  
		if (pcb[i].pid == pid && pcb[i].status != TASK_EXITED) {
			flag = 1;
			break;
	}
	}
	if (flag == 0) // 如果符合要求的进程不存在，退出并返回-1
		return -1;

	pcb_t *to_kill;
	to_kill = &pcb[i];
	if (to_kill->status == TASK_BLOCKED)    // 如果需要被杀死的进程正被阻塞，则将其从阻塞队列中释放
		queue_remove((queue_t *)(to_kill->block_me), to_kill);
  else if(to_kill->status == TASK_READY)
		queue_remove(&ready_queue, to_kill);   /*要被杀死的进程可能在readyqueue中*/
  else
    {queue_remove(&ready_queue, to_kill);
    do_scheduler();
    }
    to_kill->status == TASK_EXITED;
	// lock blocked
	while (!queue_is_empty(&(to_kill->lock_queue)))// 释放所有被持有的锁，同时释放因为得不到这些锁被阻塞的进程
   {
		mutex_lock_t *t;
		t = to_kill->lock_queue.head;
		do_unblock_all(&(t->blocked));
		queue_dequeue(&(to_kill->lock_queue));
		t->status = UNLOCKED;
	}
	//wait
	do_unblock_all(&(to_kill->wait_queue));// 等待队列清空
	to_kill->status = TASK_EXITED;
	to_kill->block_me = NULL;

	return 0;


}

int do_waitpid(pid_t pid)
{
//
	int i, find;
	find = 0;
	for (i = 0; i < NUM_MAX_TASK; i++)// 寻找需要 wait 的进程
		if (pcb[i].pid == pid) 
    {
			find = 1;
			break;
		}
	if (find == 0 && i == NUM_MAX_TASK - 1)
		return -1;               // 找不到需要wait的进程

	if (pcb[i].status != TASK_EXITED)
		do_block(&(pcb[i].wait_queue));   //将当前进程加入 to_wait 的阻塞队列 wait_queue
}

// process show
void do_process_show()
{
	kprintf("[PROCESS TABLE]\n");
	int i, num_ps = 0;
	for (i = 0; i < NUM_MAX_TASK; i++) // show running
	{
		task_status_t status = pcb[i].status;
		switch (status) {
		case TASK_RUNNING:
			kprintf("[%d] PID : %d STATUS : RUNNING\n", num_ps++,
			       pcb[i].pid);
			break;
		case TASK_READY:
			kprintf("[%d] PID : %d STATUS : READY\n", num_ps++,
			       pcb[i].pid);
			break;
		case TASK_BLOCKED:
			kprintf("[%d] PID : %d STATUS : BLOCKED\n", num_ps++,
			       pcb[i].pid);
			break;
		default:
			break;
		}
	}
}

pid_t do_getpid()
{
	return current_running->pid;
}