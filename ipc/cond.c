#include "cond.h"
#include "lock.h"

/* clear queue */
void do_condition_init(condition_t *condition)
{
    queue_init(&condition->block_queue);
}

/* The lock has been obtained before the task calls the function */
void do_condition_wait(mutex_lock_t *lock, condition_t *condition)
{
    //
    /* [1] release the lock so other task can operate on the condition */
    do_mutex_lock_release(lock);

    /* [2] do scheduler */
   do_block(&condition->block_queue);

    /* [3] acquire lock */
    do_mutex_lock_acquire(lock);
}

/* unblock one task */
void do_condition_signal(condition_t *condition)
{
     do_unblock_one(&condition->block_queue);
     //release a task for the conditon
}

/* unblock all task */
void do_condition_broadcast(condition_t *condition)
{
     do_unblock_all(&condition->block_queue);
     //release all task for the conditon
}