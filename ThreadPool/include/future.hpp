/******************************************************************************
*		Descriptions - future		      				    *
*		Date: Sun 04 Apr 2021 15:02:22 IDT					*
*		Group: RD96						      				*
*******************************************************************************/

#ifndef ILRD_RD96_FUTURE_HPP
#define ILRD_RD96_FUTURE_HPP

#include <boost/thread/mutex.hpp> //boost::mutex
#include <boost/utility.hpp> //boost::noncopyable
#include <boost/thread/condition_variable.hpp> //boost::condition_variable

namespace ilrd
{

template <class T>
class Future: private boost::noncopyable
{
public:
    Future();
    Future& Set(const T& result);
    ~Future();
    bool IsValid() const;
    T get();    
    void wait() const;
private:
    bool m_isFinished;
    T m_future;
    mutable boost::mutex m_lock;
    mutable boost::condition_variable m_isFinishedCond;

}; //Future<T>

template <>
class Future<void>: private boost::noncopyable
{
public:
    Future();
    ~Future();
    Future& Set(); // active this function when you finish the task 
    bool IsValid() const;
    void get();    
    void wait() const;
private:
    bool m_isFinished;

    mutable boost::mutex m_lock;
    mutable boost::condition_variable m_isFinishedCond;    
}; //Future<>

//*-----------------------------class Future<T>

template< class T >
Future<T>::Future()
    :m_isFinished(false),
    m_future()
{
    //empty
}

template< class T >
Future<T>::~Future()
{

}

template <class T >
Future<T>& Future<T>::Set(const T& result)
{
    m_future = result;
    {
        boost::lock_guard<boost::mutex> lock(m_lock);
        m_isFinished = true;
    }
    m_isFinishedCond.notify_all();

    return *this;
}

template <class T >
bool Future<T>::IsValid() const
{
    boost::lock_guard<boost::mutex> lock(m_lock);
    if(m_isFinished)
    {
        return true;
    }
    else
    {
        return false;
    }
}

template <class T >
void Future<T>::wait() const
{
    boost::unique_lock<boost::mutex> lock(m_lock);
    while(!m_isFinished)
    {
        m_isFinishedCond.wait(lock);
    }
}

template <class T >
T Future<T>::get()
{
    wait();
    return m_future;
}    



}//namespace ilrd


#endif   /*ILRD_RD96_FUTURE_HPP*/