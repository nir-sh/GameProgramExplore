/*******************************************************************************
--------------------------------------------------------------------------------
*	file name: stack.c
*	author: Nir Shaulian
*	reviewer: Mighty Dor Glaubach

--------------------------------------------------------------------------------
*******************************************************************************/
	/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~includes~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#include <stdlib.h> /* malloc, size_t */
#include <assert.h> /* assert */

#include "stack.h"

	/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~definitions~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

struct stack
{
    void **top;
    size_t capacity;
    void **base;
}; 

	/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~functions~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

stack_t *StackCreate(size_t capacity)
{
	stack_t *new_stack = (stack_t *)malloc(sizeof(stack_t) +\
										  (sizeof(void *) * capacity));
	assert(0 < capacity);
	if(new_stack)
	{
		new_stack->capacity = capacity;
		new_stack->base = (void **)((char*)new_stack + (sizeof(stack_t)));
		new_stack->top = new_stack->base;
	}
	return new_stack;
}

void StackDestroy(stack_t *stack)
{
	assert(stack);
	free(stack);
}

void StackPush(stack_t *stack, void *element)
{
	assert(stack);
	assert(StackSize(stack) < StackCapacity(stack));
	
	*(stack->top) = element;
	stack->top += 1;
}

void StackPop(stack_t *stack)
{
	assert(stack);
	assert(1 != StackIsEmpty(stack));
	
	stack->top -= 1;
}

void *StackPeek(const stack_t *stack)
{
	assert(stack);
	assert(1 != StackIsEmpty(stack));

	return *((stack->top)-1);
}

size_t StackSize(const stack_t *stack)
{
	assert(stack);

	return (size_t)(stack->top - stack->base);
}

int StackIsEmpty(const stack_t *stack)
{
	assert(stack);

	return !((size_t)(stack->top - stack->base));
}

size_t StackCapacity(const stack_t *stack)
{
	assert(stack);

	return stack->capacity;
}
