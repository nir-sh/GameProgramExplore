//*****************************************************************************
//*		Descriptions - Thread Safe Priority Queue API functions	    				     	  
//*		Group: RD96						 
//      Author: Nir Shualian
//      Reviewer: Coral Fakshian     								   
//******************************************************************************

#ifndef ILRD_RD96_WAITABLE_QUEUE_HPP
#define ILRD_RD96_WAITABLE_QUEUE_HPP

#include <queue>
#include <boost/utility.hpp> //boost::noncopyable
#include <boost/thread/mutex.hpp> // boost::mutex
#include <boost/thread/condition_variable.hpp> //boost::condition_variable
#include <boost/chrono.hpp> //boost::chrono
#include <boost/thread/lock_guard.hpp> // boost::lock_guard


namespace ilrd
{

template <class TYPE, class QUEUE> 
class WaitableQueue : private boost::noncopyable
{
public:

    void Enqueue(const TYPE& element);
    
    void Dequeue(TYPE *outparam);
    template <class REP, class PERIOD>
    bool Dequeue(TYPE *outparam, boost::chrono::duration<REP, PERIOD> duration);
    
    bool IsEmpty() const;
    
private:
    QUEUE m_queue;
    mutable boost::mutex m_funcLock;
    boost::condition_variable m_accessCond;

};

//******************** class WaitableQueue *******************************
//----------------------Public methods ---------------------

template <class TYPE, class QUEUE> 
void WaitableQueue<TYPE, QUEUE>::Enqueue(const TYPE& element)
{
    {
        boost::lock_guard<boost::mutex> lock(m_funcLock);
        m_queue.push(element);
    }
    m_accessCond.notify_one();
}

template <class TYPE, class QUEUE> 
void WaitableQueue<TYPE, QUEUE>::Dequeue(TYPE *outparam)
{
    boost::unique_lock<boost::mutex> lock(m_funcLock);
    while(m_queue.empty())
    {
        m_accessCond.wait(lock);
    }
    *outparam = m_queue.front();
    m_queue.pop();
}

template <class TYPE, class QUEUE> 
template <class REP, class PERIOD>
bool WaitableQueue<TYPE, QUEUE>::Dequeue(TYPE *outparam, boost::chrono::duration<REP, PERIOD> duration)
{
    boost::unique_lock<boost::mutex> lock(m_funcLock);
    while(m_queue.empty())
    {
        if (boost::cv_status::timeout == m_accessCond.wait_for(lock, duration))
        {
            return false;
        }
    }
    *outparam = m_queue.front();
    m_queue.pop();
    
    
    return true;
}

template <class TYPE, class QUEUE> 
bool WaitableQueue<TYPE, QUEUE>::IsEmpty() const
{
    boost::lock_guard<boost::mutex> lock(m_funcLock);
    return m_queue.empty();
}


} //end namespace ilrd

#endif   /*ILRD_RD96_WAITABLE_QUEUE_HPP*/