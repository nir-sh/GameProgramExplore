/*******************************************************************************
--------------------------------------------------------------------------------
*	file name: test_thread_pool.cpp
*	author: Nir Shaulian
*	reviewer:

--------------------------------------------------------------------------------
*******************************************************************************/
	/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~includes~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
#include <iostream>
#include <cassert>
#include <unistd.h> //sleep

#include <boost/thread/lock_guard.hpp>  //mutex, lock_guard
#include <boost/thread/thread.hpp> //this_thread

#include "thread_pool.hpp"

	/*~~~~~~~~~~~~~~~~~~~~~~~~~~~definitions~~~~~~~~~~~~~~~~~~~~~~~~~~*/
const char *RED_B = "\033[01;31m";
const char *GREEN_B = "\033[01;32m";
const char *YELLOW_B = "\033[01;33m";
const char *BLUE_B = "\033[01;34m";
const char *BLACK_B = "\033[01;30m";
const char *WHITE_B = "\033[01;37m";
const char *RED = "\033[0;31m";
const char *GREEN = "\033[0;32m";
const char *YELLOW = "\033[0;33m";
const char *RESET = "\033[0m";
void RunTest(bool result, const char *name)
{
    std::cout << YELLOW_B << "Test "  << name << ": " << RESET;
    if (result)
    {
        std::cout << GREEN_B << "SUCCESS" << RESET << std::endl;
    }
    else
    {
        std::cout << RED_B << "FAILURE" << RESET << std::endl;
    }
}

  ////////////////////////////////////////////////////////
 /*////////////////*/using namespace ilrd;//////////////
//////////////////////////////////////////////////////
struct ThreadId
{
    boost::thread::id *arr;
    size_t num;
};

	/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~functions~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
int TestCtor();
int TestAddTask();
int TestSetNum();
static int TestPause();
static int TestSetNum2();
static int TestsetNumSleep();
static int TestsetNumPause();
static int TestPriority();
static int TestShutdownInPause();
static int TestFuture();
static int TestFutureSimple();


static void *WriteThreadId(void *param);
static void *PrintNum(void *n);
static void *Sleep(void *);
static void *Add1(void *num);
static int Add1Int(int num);


//////////////////////////////////////////////////////////////////////////////
//---------------------MAIN
int main()
{
    TestCtor();
    TestAddTask();
    TestPause();

    TestShutdownInPause();
    TestSetNum();
    for(int i = 0; i < 20; ++i)
    {
        TestSetNum2();
    }
    TestsetNumPause();
    // TestsetNumSleep(); //takes time
    // TestPriority(); //takes time
    TestFutureSimple();
    TestFuture();

	return 0;
}
//////////////////////////////////////////////////////////////////////////////

int TestCtor()
{
    ThreadPool tp;

    return 0;
}

int TestAddTask()
{
    ThreadPool tp(0,5);
    ThreadPool::task_t print(new ThreadPool::TaskTemplate<void*, void*>(&PrintNum));
    tp.AddTask(print, ThreadPool::HIGH);
    
    Sleep((void *)1);

    return 0;    
}

static int TestPause()
{
    ThreadPool tp(0,5);

    //------Pause
    tp.Pause();
    boost::shared_ptr< Future<int> > add1_result(new Future<int>);
    ThreadPool::task_t add1(new ThreadPool::TaskWithFuture<int, int>(add1_result, &Add1Int, 1));
    
    tp.AddTask(add1, ThreadPool::HIGH);

    //-----Resume
    tp.Resume();

    //----check for result
    assert(2 == add1_result->get());

    return 0;    
}

int TestSetNum()
{
    static const int NUM_TASKS = 20;
    
    ThreadPool tp(0, 20);
    tp.SetThreadNum(1);

    boost::thread::id arr[NUM_TASKS];
    ThreadId ti_arr[NUM_TASKS];
    for(int i = 0; i < NUM_TASKS; ++i)
    {
        ti_arr[i].arr = arr;
        ti_arr[i].num = i;
        ThreadPool::task_t writeId(new ThreadPool::TaskTemplate<void*, void*>(&WriteThreadId, ti_arr + i));

        tp.AddTask(writeId, ThreadPool::HIGH);
    }
    Sleep((void *)1);
    for(int i = 0; i < NUM_TASKS; ++i)
    {
        for(int j = i; j < NUM_TASKS; ++j)
        {
            // std::cout << arr[i] << " " << arr[j] << std::endl;    
            assert(arr[i] == arr[j]);
        }
    }

    return 0; 

}
static int TestSetNum2()
{
    ThreadPool tp(0,5);
    tp.SetThreadNum(1);
    return 0;    
}

static int TestsetNumPause()
{
    using boost::shared_ptr;
    typedef Future<void*>                   future_void_pt;
    typedef shared_ptr<future_void_pt>      future_void_pt_ptr;

    ThreadPool tp(0,10);
    tp.Pause();
    for(int i = 0; i < 10; i++)
    {
        tp.SetThreadNum(1);    
    }
    tp.Resume();


    static const int NUM_TASKS = 20;
    std::vector<future_void_pt_ptr> futureArr;
    for(size_t i = 0; i < NUM_TASKS; i++)
    {
        futureArr.push_back(future_void_pt_ptr(new future_void_pt));
    }

    boost::thread::id arr[NUM_TASKS];
    ThreadId ti_arr[NUM_TASKS];
    for(int i = 0; i < NUM_TASKS; ++i)
    {
        ti_arr[i].arr = arr;
        ti_arr[i].num = i;
        ThreadPool::task_t writeId(
                new ThreadPool::TaskWithFuture<void*, void*>(
                    futureArr[i], &WriteThreadId, ti_arr + i));

        tp.AddTask(writeId, ThreadPool::HIGH);
    }

    for(size_t i = 0; i < NUM_TASKS; ++i)
    {
        futureArr[i]->get();
    }

    //check all 
    boost::thread::id val = arr[0];
    for(int i = 0; i < NUM_TASKS; ++i)
    {
        assert(arr[i] == val);
    }

    return 0;
}

static void *PrintNum(void *n)
{
    static boost::mutex mutex;
    boost::lock_guard<boost::mutex> lock(mutex);

    std::cout << (size_t)(n) << std::endl;
    std::cout << boost::this_thread::get_id() << std::endl;

    return NULL;
}

static void *WriteThreadId(void *param)
{
    ThreadId *ti = static_cast<ThreadId *>(param);
    ti->arr[ti->num] = boost::this_thread::get_id();

    return NULL;

}

static int TestsetNumSleep()
{
    static const int NUM_TASKS = 8;
    static const int NUM_THREADS = NUM_TASKS/2;
    ThreadPool tp(0, NUM_THREADS);

    for(int i = 0; i < NUM_THREADS; ++i)
    {
    
        ThreadPool::task_t sleep_task(new ThreadPool::TaskTemplate<void*, void*>(&Sleep, ((void *)1)));

        // ThreadPool::Task sleep_task(&Sleep, ((void *)1));
        tp.AddTask(sleep_task, ThreadPool::HIGH);
    }

    tp.SetThreadNum(1);
    
    boost::thread::id arr[NUM_TASKS];
    for(int i = 0; i < NUM_TASKS; ++i)
    {
        static ThreadId ti_arr[NUM_TASKS];
        ti_arr[i].arr = arr;
        ti_arr[i].num = i;

        ThreadPool::task_t writeId (new ThreadPool::TaskTemplate<void*, void*>(&WriteThreadId, ti_arr + i));
        // ThreadPool::Task writeId(&WriteThreadId, ti_arr + i);
        tp.AddTask(writeId, ThreadPool::HIGH);
    }
    Sleep((void *)1);
    // tp.Shutdown(); //can't work because some tasks paused before ending
    
    std::cout << "start compare\n";
    for(int i = 0; i < NUM_TASKS; ++i)
    {
        for(int j = i; j < NUM_TASKS; ++j)
        {
            // std::cout << arr[i] << " " << arr[j] << std::endl;    
            assert(arr[i] == arr[j]);
        }
    }
    return 0;     
}

static int TestPriority()
{
    static const int NUM_TASKS = 20;
    static const int NUM_THREADS = NUM_TASKS/2;
    ThreadPool tp(15, NUM_THREADS);

    for(int i = 0; i < NUM_THREADS; ++i)
    {
        ThreadPool::task_t sleep_task(new ThreadPool::TaskTemplate<void*, void*>(&Sleep, ((void *)30)));

        // ThreadPool::Task sleep_task(&Sleep, ((void *)30));
        tp.AddTask(sleep_task, ThreadPool::HIGH);
    }

    return 0;
}

static int TestShutdownInPause()
{
    ThreadPool tp(0,5);
    tp.Pause();

    return 0;
}

static int TestFutureSimple()
{
    boost::shared_ptr< Future<void*> > future(new Future<void*>);
    ThreadPool tp;
    size_t i = 0;

    ThreadPool::task_t task
                    (new ThreadPool::TaskWithFuture<void*, void*>
                    (future, &Add1, reinterpret_cast<void*>(i)));

    // ThreadPool::Task task(Add1, reinterpret_cast<void*>(i));
    tp.AddTask(task, ThreadPool::HIGH);

    assert((i + 1) == reinterpret_cast<size_t>(future->get()));

    return 0;

}



static int TestFuture()
{
    using boost::shared_ptr;
    typedef Future<void*>                   future_void_pt;
    typedef shared_ptr<future_void_pt>      future_void_pt_ptr;
    const size_t ARR_SIZE = 100;

    std::vector<future_void_pt_ptr> futureArr;
    for(size_t i = 0; i < ARR_SIZE; i++)
    {
        futureArr.push_back(future_void_pt_ptr(new future_void_pt));
    }

    ThreadPool tp;

    for(size_t i = 0; i < ARR_SIZE; ++i)
    {
        ThreadPool::task_t task
                        (new ThreadPool::TaskWithFuture<void*, void*>
                        (futureArr[i], &Add1, reinterpret_cast<void*>(i)));

        // ThreadPool::Task task(Add1, reinterpret_cast<void*>(i));
        tp.AddTask(task, ThreadPool::HIGH);
    }

    for(size_t i = 0; i < ARR_SIZE; ++i)
    {
        assert((i + 1) == reinterpret_cast<size_t>(futureArr[i]->get()));
    }

    return 0;
}

static void *Sleep(void *time)
{
    std::cout << "sleeping for " << (size_t)time << std::endl;
    sleep((size_t)time);

    return NULL;
}

static void *Add1(void *num)
{
    long retNum = reinterpret_cast<size_t>(num);

    return reinterpret_cast<void*>(++retNum);
}

static int Add1Int(int num)
{
    return ++num;
}