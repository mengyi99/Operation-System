#include "barrier.h"

void do_barrier_init(barrier_t *barrier, int goal)
{
    barrier->goal = goal;
    barrier->now = 0;
}

void do_barrier_wait(barrier_t *barrier)
{
    barrier->now++;    //增大当前等待任务
    if (barrier->now < barrier->goal)   //小于增加
    {
        do_block(&barrier->block_queue);
    } 
    else                                //释放
    {
        do_unblock_all(&barrier->block_queue);
        barrier->now = 0;
    }   
}