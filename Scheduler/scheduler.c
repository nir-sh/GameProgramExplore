/*******************************************************************************
--------------------------------------------------------------------------------
*	file name: scheduler.c
*	author: Nir Shaulian
*	reviewer: Lia Borisover

--------------------------------------------------------------------------------
*******************************************************************************/
	/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~includes~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
#include <assert.h> /* assert */
#include <stdlib.h> /*malloc, free */
#include <unistd.h> /* sleep */

#include "priority_queue.h" /*PQ funcs */
#include "task.h" /*Task funcs */

#include "scheduler.h"
	/*~~~~~~~~~~~~~~~~~~~~~~~~~~~definitions~~~~~~~~~~~~~~~~~~~~~~~~~~*/
struct scheduler
{
    pq_t *p_queue;
    task_t *curr_task;
    int is_running;
};
enum matching {NO, YES};
enum successful {SUCCEED, FAILED};

#define SEC_TO_EXECUTE(task)(\
        (start_time + TaskGetTimeToExecute(task) - now) > 0 ?\
        (start_time + TaskGetTimeToExecute(task) - now) : 0)
	/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~static functions~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

static int prior_func(const void *one, const void *other)
{
    return TaskGetTimeToExecute((task_t *)one) -
            TaskGetTimeToExecute((task_t *)other);
}

static int UIDIsSameWraper(const void *one, const void *other, size_t n)
{
    n = n;
    return UIDIsSame(TaskGetUID((task_t *)one), *((ilrd_uid_t *)other));
}
    /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~functions~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

sched_t *SchedCreate(void)
{
    sched_t *new_sched = (sched_t *)malloc(sizeof(sched_t));
    if(NULL == new_sched)
    {
        return NULL;
    }
    new_sched->p_queue = PQCreate(prior_func);
    if(NULL == new_sched->p_queue)
    {
        free(new_sched);
        return NULL;
    }
    new_sched->is_running = 0;

    return new_sched;
}

void SchedDestroy(sched_t *sched)
{
    assert(NULL != sched);

    SchedFlush(sched);
    PQDestroy(sched->p_queue);
    free(sched); sched = NULL;
}

ilrd_uid_t SchedAddTask(sched_t *sched, time_t time_to_execute, time_t frequency
								 , int(*oper_func)(void *params), void *params)
{
    task_t *new = NULL;

    assert(NULL != sched);

    new = TaskCreate(time_to_execute, frequency, oper_func, params);
    if(NULL == new)
    {
        return UIDGetBadUID();
    }
    if(FAILED == PQEnqueue(sched->p_queue, new))
    {
        TaskDestroy(new);
        return UIDGetBadUID();
    }

    return TaskGetUID(new);
}

void SchedRemoveTask(sched_t *sched, ilrd_uid_t uid)
{
    assert(NULL != sched);

    TaskDestroy(PQErase(sched->p_queue, UIDIsSameWraper, &uid, sizeof(int)));
}

void SchedFlush(sched_t *sched)
{
    assert(NULL != sched);

    while(NO == SchedIsEmpty(sched))
    {
        TaskDestroy(PQPeek(sched->p_queue));
        PQDequeue(sched->p_queue);
    }
}

int SchedRun(sched_t* sched)
{
    time_t start_time = 0;
    time_t time_to_sleep = 0;
    time_t now = 0;
    int status = SUCCEED;

    assert(NULL != sched);

    for(start_time = time(NULL); -1 == start_time; start_time = time(NULL));

    sched->is_running = YES;
    while(YES == sched->is_running && NO == SchedIsEmpty(sched))
    {
        /*protection for time() */
        for(now = time(NULL); -1 == now; now = time(NULL));

        sched->curr_task = PQPeek(sched->p_queue);
        PQDequeue(sched->p_queue);
        /*protection for sleep() */
        for (time_to_sleep = SEC_TO_EXECUTE(sched->curr_task);
            0 != time_to_sleep;
            time_to_sleep = sleep(time_to_sleep))
        {
            /* empty loop */
        }
        status = TaskRun(sched->curr_task);

        if(0 < TaskGetFrequency(sched->curr_task))
        {
            TaskReschedule(sched->curr_task);
            if(FAILED == PQEnqueue(sched->p_queue, sched->curr_task))
            {
                TaskDestroy(sched->curr_task);

                return FAILED; sched->curr_task = NULL;
            }
        }
        else
        {
            TaskDestroy(sched->curr_task); sched->curr_task = NULL;
        }
        if (FAILED == status)
        {
            return FAILED;
        }
    }

    return SUCCEED;
}


int SchedStop(sched_t *sched)
{
    assert(NULL != sched);

    sched->is_running = NO;

    return SUCCEED;
}

int SchedIsEmpty(sched_t* sched)
{
    assert(NULL != sched);

    return PQIsEmpty(sched->p_queue);
}

size_t SchedSize(sched_t* sched)
{
    assert(NULL != sched);

    return PQSize(sched->p_queue);
}
