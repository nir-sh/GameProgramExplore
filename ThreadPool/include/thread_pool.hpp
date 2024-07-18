//*****************************************************************************
//*		Descriptions - Thread Pool API functions	    				     	  
//*		Group: RD96						      								   
//******************************************************************************
#ifndef ILRD_RD96_THREAD_POOL_HPP
#define ILRD_RD96_THREAD_POOL_HPP

#include <utility> //pair

#include <boost/utility.hpp> //boost::noncopyable
#include <boost/thread.hpp> //boost::thread, hardware_concurrency	
#include <boost/function.hpp> //boost::function
#include <boost/smart_ptr/shared_ptr.hpp> //boost::shared_ptr
#include <boost/atomic.hpp> //atomic
#include <boost/thread/mutex.hpp> //mutex
#include <boost/thread/condition_variable.hpp> //boost::condition_variable
#include <boost/thread/latch.hpp> //latch //TODO:

#include "future.hpp" //future
#include "priority_queue_wrapper.hpp"
#include "waitable_queue.hpp" 
#include "thread_group.hpp"


namespace ilrd
{

class ThreadPool : private boost::noncopyable
{
public:
    //------------Declerations
    class Task;
    typedef boost::shared_ptr<Task>                           task_t;
    enum Priority 
    {
        LOW = 0,
        MED = 1,
        HIGH = 2
    };

    //-------------Methods
    ThreadPool(int niceness = 0, size_t num_threads =
                boost::thread::hardware_concurrency());
    ~ThreadPool();
    void AddTask(task_t task, Priority priority);
    void SetThreadNum(size_t size);

    void Pause();
    void Resume();
    void Shutdown();

    //-------------Task(interface)
    class Task
    {
    public:
        virtual void Run() = 0;
        virtual ~Task() = 0;

    };

    //-------------TaskTemplate
    template<typename R, typename P>
    class TaskTemplate: public Task
    {
    public:
        typedef boost::function<R(P)> task_ptr;
        TaskTemplate(task_ptr task = NULL, P param = NULL);
        virtual void Run();
        virtual ~TaskTemplate();
    private:
        task_ptr m_task;
        P m_param;
        
    };
    
    //-------------Task With Future
    template<typename R, typename P>
    class TaskWithFuture : public Task
    {
    public:
        typedef boost::function<R(P)> task_ptr;
        typedef boost::shared_ptr<Future<R> >                   future_t;

        TaskWithFuture(future_t future, task_ptr task = NULL, P param = NULL);
        void Run();
    private:
        task_ptr m_task;
        P m_param;
        future_t m_future; 
    };
    

    //--Task With Future (void return value case)
    template<typename P>
    class TaskWithFuture<void, P> : public Task
    {
    public:
        typedef void(*task_ptr)(P);
        typedef boost::shared_ptr<Future<void> >                   future_t;

        TaskWithFuture(future_t future, task_ptr task = NULL, P param = NULL);
        void Run();
    private:
        task_ptr m_task;
        P m_param;
        future_t m_future; 
    };
    
private:
    enum
    {
        TOP_PRIORITY = HIGH + 1
    };
    class Compare;
    typedef std::pair<task_t, int>                             prioritized_task;
    typedef Priority_Queue< prioritized_task,
            std::vector<prioritized_task>, Compare >           PQueue;
    typedef WaitableQueue<prioritized_task, PQueue>            WQueue;
    typedef boost::shared_ptr<boost::thread>                   thread_ptr;
    typedef ThreadGroup                                        thread_group;
    typedef boost::latch                                       latch;
    class Compare
    {
    public:
        bool operator()(const prioritized_task& lhs,
                        const prioritized_task& rhs) const;
    };

    //--------private members
    bool m_paused;
    bool m_shutdown;
    int m_niceness;
    size_t m_numThreads;
    WQueue m_waitableQueue;
    mutable boost::mutex m_poolLock;
    // boost::condition_variable m_pauseCond;
    mutable boost::mutex m_shutdownLock;
    thread_group m_threads;
    latch m_pauseLatch;

    //--------private methods
    void ManageThread();
    // void PauseRaw();
    void AddThreadRaw();
    void AddMultipleThreads(size_t numThreads);
    void AddTaskRaw(task_t task, int priority);

};//class ThreadPool
//******************************************************************************

//****************************Class TaskTemplate
template<typename R, typename P>
ThreadPool::TaskTemplate<R,P>::TaskTemplate(task_ptr task, P param)
	:m_task(task), m_param(param)
{
	//empty
}

template<typename R, typename P>
void ThreadPool::TaskTemplate<R,P>::Run()
{
	m_task(m_param);
}

template<typename R, typename P>
ThreadPool::TaskTemplate<R,P>::~TaskTemplate()
{
	//empty
}

//****************************Class TaskWithFuture
template<typename R, typename P>
ThreadPool::TaskWithFuture<R,P>::TaskWithFuture(future_t future,
                                                task_ptr task,
                                                P param)
	:m_task(task), m_param(param), m_future(future)										
{

}										

template<typename R, typename P>
void ThreadPool::TaskWithFuture<R,P>::Run()
{
	m_future->Set(m_task(m_param));
}
//*--------------------TaskWithFuture<void>
template<typename P>
ThreadPool::TaskWithFuture<void,P>::TaskWithFuture(future_t future,
                                                task_ptr task,
                                                P param)
	:m_task(task), m_param(param), m_future(future)										
{
    //empty
}										

template<typename P>
void ThreadPool::TaskWithFuture<void,P>::Run()
{
    m_task(m_param);
	m_future->Set();
}


} //end namespace ilrd

#endif   /*ILRD_RD96_THREAD_POOL_HPP*/
