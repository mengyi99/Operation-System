
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *  * * * * * * * * * * *
 *            Copyright (C) 2018 Institute of Computing Technology, CAS
 *               Author : Han Shukai (email : hanshukai@ict.ac.cn)
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *  * * * * * * * * * * *
 *         The kernel's entry, where most of the initialization work is done.
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *  * * * * * * * * * * *
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this
 * software and associated documentation files (the "Software"), to deal in the Software
 * without restriction, including without limitation the rights to use, copy, modify,
 * merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit
 * persons to whom the Software is furnished to do so, subject to the following conditions:
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

#include "fs.h"
#include "irq.h"
#include "test.h"
#include "stdio.h"
#include "sched.h"
#include "screen.h"
#include "common.h"
#include "syscall.h"
#include "smp.h"
#include "mm.h"
#include "mac.h"
#include "string.h"

#define TASK_INIT (00)

static void init_memory()
{
}

static void init_pcb()
{
   process_id = 0; // kernel process

    // Init
    int i;
    for (i = 1; i < NUM_MAX_TASK; i++)
    {
        pcb[i].status = TASK_EXITED;
    }
    queue_init(&ready_queue);
    queue_init(&block_queue);
    // pcb0
    pcb[0].kernel_stack_top = STACK_TOP; //sp
    pcb[0].user_stack_top = STACK_USER_TOP;
    pcb[0].block_me = NULL;
    pcb[0].prev = NULL;
    pcb[0].next = &pcb[1];
    set_pcb(0, &pcb[0], &shell_task);
    queue_push(&ready_queue, &pcb[0]);
    
    pcb[0].cursor_x = SHELL_LEFT_LOC;
    pcb[0].cursor_y = SHELL_BOUNDARY;
    pcb[0].status = TASK_RUNNING;

    pcb[1].kernel_stack_top = STACK_TOP + STACK_SIZE; //sp
    pcb[1].user_stack_top = STACK_USER_TOP + STACK_SIZE;
    pcb[1].block_me = NULL;
    pcb[1].prev = NULL;
    pcb[1].next = &pcb[0];
    pcb[1].kernel_context.regs[29] = pcb[1].kernel_stack_top;
    pcb[1].user_context.regs[29] = pcb[1].user_stack_top;

    current_running = &pcb[1];

}


static void init_exception_handler()
{
}

static void init_exception()
{

    /* fill nop */

    /* fill nop */

    /* set COUNT & set COMPARE */

    /* open interrupt */
}

// [2]
// extern int read_shell_buff(char *buff);

static void init_syscall(void)
{
}

/* [0] The beginning of everything */
void __attribute__((section(".entry_function"))) _start(void)
{

    asm_start();

    /* init stack space */
    init_stack();
    printk("> [INIT] Stack initialization succeeded.\n");

    /* init interrupt */
 //   init_exception();
 //   printk("> [INIT] Interrupt processing initialization succeeded.\n");

  //  init_memory();
  //  printk("> [INIT] Virtual memory initialization succeeded.\n");

    /* init system call table */
 //   init_syscall();
 //   printk("> [INIT] System call initialized successfully.\n");

    /* init Process Control Block */
    init_pcb();
    printk("> [INIT] PCB initialization succeeded.\n");

    /* init screen */
    init_screen();
    printk("> [INIT] SCREEN initialization succeeded.\n");

    /* init filesystem */
    read_super_block();

    /* wake up core1*/
    //?loongson3_boot_secondary();

    /* set cp0_status register to allow interrupt */
    // enable exception and interrupt
    // ERL = 0, EXL = 0, IE = 1

    
    while (1)
    {
        do_scheduler(); 
    };
    return;
}
