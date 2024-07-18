/******************************************************************************
*		Descriptions - Thread Group		      				  *
*		Date: Tue 30 Mar 2021 19:09:04 IDT					     				*
*		Group: RD96						      				*
*******************************************************************************/

#ifndef ILRD_RD96_THREAD_GROUP_HPP
#define ILRD_RD96_THREAD_GROUP_HPP

#include <list>
#include <boost/thread/csbl/memory/unique_ptr.hpp>
#include <boost/thread/shared_mutex.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/lock_guard.hpp>
#include <boost/thread/thread.hpp>

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
namespace ilrd
{

class ThreadGroup
{
private:
    ThreadGroup(ThreadGroup const&);
    ThreadGroup& operator=(ThreadGroup const&);
public:
    typedef boost::thread                                       thread;
    typedef std::list<thread*>                                  thread_list;

    ThreadGroup()
    {
        //empty
    }

    ~ThreadGroup()
    {
        RemoveAll();
    }

    bool IsThisThreadIn()
    {
        thread::id id = boost::this_thread::get_id();
        boost::shared_lock<boost::shared_mutex> guard(m);
        for(thread_list::iterator it=m_threads.begin(),end=m_threads.end();
            it!=end;
            ++it)
        {
            if ((*it)->get_id() == id)
            return true;
        }
        return false;
    }

    bool IsThreadIn(thread* thrd)
    {
        if(NULL != thrd)
        {
            thread::id id = thrd->get_id();
            boost::shared_lock<boost::shared_mutex> guard(m);
            for(thread_list::iterator it=m_threads.begin(),end=m_threads.end();
                it!=end;
                ++it)
            {
                if ((*it)->get_id() == id)
                return true;
            }
            return false;
        }
        else
        {
            return false;
        }
    }

    template<typename F>
    thread* CreateThread(F threadfunc)
    {
        boost::lock_guard<boost::shared_mutex> guard(m);
        return CreateThreadRaw(threadfunc);
    }

    template<typename F>
    void CreateMultipleThreads(F threadfunc, size_t num)
    {
        boost::lock_guard<boost::shared_mutex> guard(m);
        for(size_t i = 0; i < num; ++i)
        {
            CreateThreadRaw(threadfunc);
        }
    }

    void RemoveThread(thread* thrd)
    {
        boost::lock_guard<boost::shared_mutex> guard(m);
        thread_list::iterator const it=std::find(m_threads.begin(),m_threads.end(),thrd);
        if(it!=m_threads.end())
        {
            delete thrd;
            m_threads.erase(it);
        }
    }

    void RemoveAll()
    {
        boost::lock_guard<boost::shared_mutex> guard(m);
        while(!m_threads.empty())
        {
            delete m_threads.front();
            m_threads.pop_front();
        }
    }

    void RemoveCurrentThread()
    {
        boost::thread::id id = boost::this_thread::get_id();
        boost::lock_guard<boost::shared_mutex> lock(m);
        thread_list::iterator i = m_threads.begin();
        for(; i != m_threads.end(); ++i)
        {
            if((*i)->get_id() == id)
            {
                delete *i;
                m_threads.erase(i);
                
                break;
            }
        }
    }

    void JoinAll()
    {
        boost::shared_lock<boost::shared_mutex> guard(m);

        for(thread_list::iterator it=m_threads.begin(),end=m_threads.end();
            it!=end;
            ++it)
        {
            if ((*it)->joinable())
            (*it)->join();
        }
    }

    void InterruptAll()
    {
        boost::shared_lock<boost::shared_mutex> guard(m);

        for(thread_list::iterator it=m_threads.begin(),end=m_threads.end();
            it!=end;
            ++it)
        {
            (*it)->interrupt();
        }
    }

    size_t Size() const
    {
        boost::shared_lock<boost::shared_mutex> guard(m);
        return m_threads.size();
    }

private:
    thread_list m_threads;
    mutable boost::shared_mutex m;


    template<typename F>
    thread* CreateThreadRaw(F threadfunc)
    {
        boost::csbl::unique_ptr<thread> new_thread(new thread(threadfunc));
        m_threads.push_back(new_thread.get());
        return new_thread.release();
    }
};

} //namespace ilrd

#endif   /*ILRD_RD96_THREAD_GROUP_HPP*/
