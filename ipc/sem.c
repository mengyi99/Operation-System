#include "sem.h"
#include "stdio.h"

void do_semaphore_init(semaphore_t *s, int val)
{
    s->semph = val;
}

void do_semaphore_up(semaphore_t *s)
{
     s->semph++;
     //val++;
     /*realease*/
     if(s->semph==0)
     {
         do_unblock_one(&s->block_queue);
     }
}

void do_semaphore_down(semaphore_t *s)
{
    s->semph--;
    if(s->semph < 0)
    {
       do_block(&s->block_queue);
    }  
    //val--
    //<0，block
}