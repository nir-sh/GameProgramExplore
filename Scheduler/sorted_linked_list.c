/*******************************************************************************
--------------------------------------------------------------------------------
*	file name: sorted_linked_list.c
*	author: Nir Shaulian
*	reviewer: Big Sam Gross

--------------------------------------------------------------------------------
*******************************************************************************/
	/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~includes~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
#include <stdlib.h> /* malloc, free */
#include <assert.h> /* assert */

#include "sorted_linked_list.h"
	/*~~~~~~~~~~~~~~~~~~~~~~~~~~~definitions~~~~~~~~~~~~~~~~~~~~~~~~~~*/

struct sorted_list
{
    cmp_func_t cmp_func;
    dl_t *dlist;
};

enum successful {SUCCEED, FAILED};
enum matching {NO, YES};
	/*~~~~~~~~~~~~~~~~~~~~~~~~~~API functions~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

sol_t *SortedListCreate(cmp_func_t cmp_func)
{
    sol_t *new_sol = (sol_t *)malloc(sizeof(sol_t));
	if(NULL == new_sol)
	{
		return NULL;
	}
    new_sol->dlist = DListCreate();
    if(NULL == new_sol->dlist)
    {
        free(new_sol);
        return NULL;
    }
    new_sol->cmp_func = cmp_func;

	return new_sol;
}

void SortedListDestroy(sol_t *solist)
{
    DListDestroy(solist->dlist);
    free(solist);
}

int SortedListIsEmpty(const sol_t *solist)
{
    assert(NULL != solist);

    return DListIsEmpty(solist->dlist);
}

size_t SortedListSize(const sol_t *solist)
{
    assert(NULL != solist);

    return DListSize(solist->dlist);
}

sol_iter_t SortedListBegin(const sol_t *solist)
{
    assert(NULL != solist);

    return DListBegin(solist->dlist);
}

sol_iter_t SortedListEnd(const sol_t *solist)
{
    assert(NULL != solist);

    return DListEnd(solist->dlist);
}

sol_iter_t SortedListNext(const sol_iter_t current)
{
    return DListNext(current);
}

sol_iter_t SortedListPrev(const sol_iter_t current)
{
    return DListPrev(current);
}

int SortedListIsSameIter(const sol_iter_t one, const sol_iter_t other)
{
    return DListIsSameIter(one, other);
}

void *SortedListGetData(const sol_iter_t iter)
{
    return DListGetData(iter);
}

int SortedListInsert(sol_t *solist, const void *data)
{
    sol_iter_t runner = NULL;

    assert(NULL != solist);

    for(runner = DListBegin(solist->dlist);
                    NO == DListIsSameIter(runner, DListEnd(solist->dlist)) &&
                    0 >= solist->cmp_func(DListGetData(runner), data);
                    runner = DListNext(runner))
    {
        /*empty loop*/
    }
    return DListInsertBefore(runner, data);
}

void SortedListRemove(sol_iter_t iter)
{
    assert(NULL != DListNext(iter));

    DListRemove(iter);
}

void SortedListPopFront(sol_t *solist)
{
    assert(NULL != solist);
    assert(NO == DListIsEmpty(solist->dlist));

    DListPopFront(solist->dlist);
}

void SortedListPopBack(sol_t *solist)
{
    assert(NULL != solist);
    assert(NO == DListIsEmpty(solist->dlist));

    DListPopBack(solist->dlist);
}

int SortedListForEach(sol_iter_t from, const sol_iter_t to,
						oper_func_t oper_func, void *params)
{
    return DListForEach(from, to, oper_func, params);
}

sol_iter_t SortedListFind(sol_t *solist, const sol_iter_t from,
            const sol_iter_t to, const void *data)
{
    sol_iter_t runner = from;

    assert(NULL != solist);

    for(runner = from;
            NO == SortedListIsSameIter(runner, to) &&
            NO == SortedListIsSameIter(runner, SortedListEnd(solist)) &&
            0 > solist->cmp_func(DListGetData(runner), data);
            runner = SortedListNext(runner))
    {
        /* empty loop */
    }
    if(NO == SortedListIsSameIter(runner, SortedListEnd(solist)) &&
        0 == solist->cmp_func(DListGetData(runner), data))
    {
        return runner;
    }

    return to;
}

sol_iter_t SortedListFindIf(const sol_iter_t from, const sol_iter_t to,
					 criter_match_t criter_match, const void *params, size_t n)
{
    return DListFind(from, to, criter_match, params, n);
}

void SortedListMerge(sol_t *dest, sol_t *src)
{
    sol_iter_t src_run = NULL;
    sol_iter_t dest_run = NULL;
    sol_iter_t from = NULL;

    assert(NULL != dest);
    assert(NULL != src);
    assert(NO == SortedListIsSameIter(SortedListBegin(dest),
                                        SortedListBegin(src)));

    src_run = DListBegin(src->dlist);
    dest_run = DListBegin(dest->dlist);
    from = src_run;

    while(NO == SortedListIsSameIter(dest_run, SortedListEnd(dest)) &&
                NO == SortedListIsSameIter(src_run, SortedListEnd(src)))
    {
        while(NO == SortedListIsSameIter(src_run, SortedListEnd(src)) &&
                            0 < dest->cmp_func(SortedListGetData(dest_run),
                                                SortedListGetData(src_run)))
        {
            src_run = SortedListNext(src_run);
        }
        DListSplice(dest_run, from, src_run);
        from = SortedListBegin(src);
        dest_run = SortedListNext(dest_run);

    }
    if(NO == SortedListIsEmpty(src))
    {
        DListSplice(dest_run, from, SortedListEnd(src));
    }
}
