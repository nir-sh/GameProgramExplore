/*******************************************************************************
--------------------------------------------------------------------------------
*	file name: thread_pool.cpp
*	author: Nir Shaulian
*	reviewer: Or the kind Yosef 

--------------------------------------------------------------------------------
*******************************************************************************/
	/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~includes~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
#include <exception> //std::exception
#include <sys/resource.h> // setpriority

#include <boost/thread/shared_mutex.hpp> // boost::shared_mutex

#include "thread_pool.hpp"

namespace ilrd
{

	/*~~~~~~~~~~~~~~~~~~~~~~~~~~~namespace details~~~~~~~~~~~~~~~~~~~~~~~~~~*/
	namespace details
	{
	static void Pause(void * = NULL);
	static void KillThread(void * = NULL);

	struct BadApple : public std::exception
	{
		//empty
	};

	}//namespace details
//*********************************Class ThreadPool****************************	
	/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~Methods~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
ThreadPool::ThreadPool(int niceness, size_t num_threads)
	:m_paused(false),
	m_shutdown(false),
	m_niceness(niceness),
    m_numThreads(0),
	m_waitableQueue(),
	m_poolLock(),
	m_shutdownLock(),
	m_threads(),
	m_pauseLatch(0)
{
	SetThreadNum(num_threads);
}

ThreadPool::~ThreadPool()
{
	if(!m_shutdown)
	{
		Shutdown();
	}
}

void ThreadPool::AddTask(task_t task, Priority priority)
{
	m_waitableQueue.Enqueue(prioritized_task(task, priority));
}

void ThreadPool::SetThreadNum(size_t size)
{
	boost::lock_guard<boost::mutex> lock(m_poolLock);
	
	if(m_numThreads < size)
	{
		AddMultipleThreads(size - m_numThreads);
	}
	//-----BadApple way
	for(size_t i = m_numThreads; i > size; --i)
	{
		task_t task(new TaskTemplate<void, void*>(details::KillThread));
		AddTaskRaw(task, TOP_PRIORITY);
	}

	m_numThreads = size;
}


void ThreadPool::Pause()
{
	boost::lock_guard<boost::mutex> lock(m_poolLock);
	if(true == m_pauseLatch.try_wait())
	{
		m_pauseLatch.reset(1);
		
		task_t task(new TaskTemplate<void, void*>(details::Pause));
		for(size_t i = 0; i < m_numThreads; ++i)
		{
			AddTaskRaw(task, TOP_PRIORITY);
		}
	}
}

void ThreadPool::Resume()
{
	m_pauseLatch.try_count_down();	
}

void ThreadPool::Shutdown()
{
	{
		//lock until all thread needed to erase have been removed
		boost::lock_guard<boost::mutex> lock(m_shutdownLock);
		m_shutdown = true;
	}
	Pause(); //get all threads out of the waitable queue
	Resume(); // get all threads out of the latch
	m_threads.JoinAll(); // to prevent use of "this" after destroy in other threads
	m_threads.RemoveAll();
	m_numThreads = 0;
}

//---------------------------Private Methods------------------------
void ThreadPool::ManageThread()
{
	setpriority(PRIO_PROCESS, gettid(), m_niceness);
	try
	{
		while(!m_shutdown)
		{
			prioritized_task task;
			m_waitableQueue.Dequeue(&task);
			task.first->Run();

			//wait if Pause() is requested
			m_pauseLatch.wait();
		}
	}//try

	catch(const details::BadApple& e)
	{
		if(!m_shutdown)
		{
			boost::lock_guard<boost::mutex> lock(m_shutdownLock);
			if(!m_shutdown)
			{
				m_threads.RemoveCurrentThread();
			}
		}
	}	
}


void ThreadPool::AddMultipleThreads(size_t numThreads)
{
	m_threads.CreateMultipleThreads(
					boost::bind(&ThreadPool::ManageThread, this), numThreads);
}

void ThreadPool::AddTaskRaw(task_t task, int priority)
{
	m_waitableQueue.Enqueue(prioritized_task(task, priority));	
}

//*************************Class Compare
bool ThreadPool::Compare::operator()(const prioritized_task& lhs,
                        const prioritized_task& rhs) const
{
    return (0 > (lhs.second - rhs.second));
}


static void details::Pause(void *)
{
	//empty
}

static void details::KillThread(void *)
{
	throw details::BadApple();

}

//****************************Class Task***********************************
ThreadPool::Task::~Task()
{
	//empty
}


}//namespace ilrd