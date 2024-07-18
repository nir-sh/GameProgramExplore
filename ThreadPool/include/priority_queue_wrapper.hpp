/******************************************************************************
*		Descriptions - priority_queue_wrapper		      				  *
*		Date: Thu 18 Mar 2021 15:23:45 IST					     				*
*		Group: RD96						      				*
*******************************************************************************/

#ifndef ILRD_RD96_PRIORITY_QUEUE_WRAPPER_HPP
#define ILRD_RD96_PRIORITY_QUEUE_WRAPPER_HPP

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
#include <queue>

namespace ilrd
{

template<
    class T,
    class Container = std::vector<T>,
    class Compare = std::less<typename Container::value_type>
    >
class Priority_Queue : private std::priority_queue<T, Container, Compare>
{
public:
    explicit Priority_Queue( const Compare& compare = Compare(),
                         const Container& cont = Container() )
        : std::priority_queue<T, Container, Compare>::priority_queue(compare, cont)
    {

    }
    // ~Priority_Queue()

    Priority_Queue& operator=( const Priority_Queue& other )
    {
        std::priority_queue<T, Container, Compare>::operator=(other);
        return *this;
    }

    typename std::priority_queue<T, Container, Compare>::const_reference front() const
    {
        return std::priority_queue<T, Container, Compare>::top();
    }

	bool empty() const
    {
        return std::priority_queue<T, Container, Compare>::empty();
    }

    typename std::priority_queue<T, Container, Compare>::size_type size() const
    {
        return std::priority_queue<T, Container, Compare>::size();
    }

	void push( const typename std::priority_queue<T, Container, Compare>::value_type& value )
    {
        std::priority_queue<T, Container, Compare>::push( value );
    }

    void pop()
    {
        std::priority_queue<T, Container, Compare>::pop();
    }

	void swap( Priority_Queue& other )
    {
        std::priority_queue<T, Container, Compare>::swap( other );
    }

private:
};


} //namespace ilrd

#endif   /*ILRD_RD96_PRIORITY_QUEUE_WRAPPER_HPP*/
