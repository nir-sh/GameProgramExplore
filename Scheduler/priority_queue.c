/*******************************************************************************
--------------------------------------------------------------------------------
*	file name: priority_queue.c
*	author: Nir Shaulian
*	reviewer: master Yoni Ladijensky

--------------------------------------------------------------------------------
*******************************************************************************/
	/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~includes~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
#include <assert.h> /* assert */
#include <stdlib.h> /* malloc, free */

#include "sorted_linked_list.h"
#include "priority_queue.h"
	/*~~~~~~~~~~~~~~~~~~~~~~~~~~~definitions~~~~~~~~~~~~~~~~~~~~~~~~~~*/
struct p_queue
{
  sol_t *solist;
};
enum matching {NO, YES};
	/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~functions~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

pq_t *PQCreate(prior_func_t prior_func)
{
    pq_t *new_pq = (pq_t *)malloc(sizeof(pq_t));
    if(NULL == new_pq)
    {
        return NULL;
    }
    new_pq->solist = SortedListCreate(prior_func);
    if(NULL == new_pq->solist)
    {
        free(new_pq);
        return NULL;
    }

    return new_pq;
}

void PQDestroy(pq_t *p_queue)
{
    SortedListDestroy(p_queue->solist);
    free(p_queue);
}

int PQIsEmpty(const pq_t *p_queue)
{
    assert(NULL != p_queue);

    return SortedListIsEmpty(p_queue->solist);
}

int PQEnqueue(pq_t *p_queue, const void *data)
{
    assert(NULL != p_queue);

    return SortedListInsert(p_queue->solist, data);
}

void PQDequeue(pq_t *p_queue)
{
    assert(NULL != p_queue);
    assert(NO == SortedListIsEmpty(p_queue->solist));

    SortedListPopFront(p_queue->solist);
}

void *PQPeek(const pq_t *p_queue)
{
    assert(NULL != p_queue);
    assert(NO == SortedListIsEmpty(p_queue->solist));

    return SortedListGetData(SortedListBegin(p_queue->solist));
}

size_t PQSize(const pq_t *p_queue)
{
    assert(NULL != p_queue);

    return SortedListSize(p_queue->solist);
}

void PQFlush(pq_t *p_queue)
{
    assert(NULL != p_queue);
    assert(NO == SortedListIsEmpty(p_queue->solist));

    for(; NO == PQIsEmpty(p_queue); PQDequeue(p_queue))
    {
        /* empty loop */
    }
}

void *PQErase(pq_t *p_queue, is_criter_match_t is_criter_match,
                const void *data, size_t n)
{
    void *get_data = NULL;
    sol_iter_t result;

    assert(NULL != p_queue);

    result = SortedListFindIf(SortedListBegin(p_queue->solist),
                                        SortedListEnd(p_queue->solist),
                                        is_criter_match, data, n);
    if(NO == SortedListIsSameIter(result, SortedListEnd(p_queue->solist)))
    {
        get_data = SortedListGetData(result);
        SortedListRemove(result);
    }

    return get_data;
}
