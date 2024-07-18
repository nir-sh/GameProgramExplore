/*******************************************************************************
--------------------------------------------------------------------------------
*	file name: future.cpp
*	author: Nir Shaulian
*	reviewer:

--------------------------------------------------------------------------------
*******************************************************************************/
	/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~includes~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
#include "future.hpp"
	/*~~~~~~~~~~~~~~~~~~~~~~~~~~~definitions~~~~~~~~~~~~~~~~~~~~~~~~~~*/
namespace ilrd
{

	/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~functions~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
//*-----------------------------class Future<void>

Future<void>::Future()
    :m_isFinished(false)
{
    //empty
}

Future<void>::~Future()
{

}

Future<void>& Future<void>::Set()
{
    boost::lock_guard<boost::mutex> lock(m_lock);
    m_isFinished = true;
    m_isFinishedCond.notify_all();

    return *this;
}

bool Future<void>::IsValid() const
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

void Future<void>::wait() const
{
    boost::unique_lock<boost::mutex> lock(m_lock);
    while(!m_isFinished)
    {
        m_isFinishedCond.wait(lock);
    }
}

void Future<void>::get()
{
    wait();
}    

}//namespace ilrd


