README file for Thread Pool
=============================

README - general information
-------------------------------

A thread pool is a software design pattern for achieving concurrency of 
execution in a program (also called worker-crew model).
 a thread pool maintains multiple threads waiting for tasks to be allocated for
concurrent execution by the supervising program.
 By maintaining a pool of threads, the model increases performance and avoids 
latency in execution due to frequent creation and destruction of threads for 
short-lived tasks.


The ThreadPool supports running the following different types of tasks:

1. Regular Task
	task which containing function to execute and priority.
  
2. Task with Future object outparameter to get the return value of the task
	synchronously


INSTALL
-------------------------------
1. Put your test_thread_pool.cpp file in the 'test' directory.

2. How to install the release version?
   run from linux command line (from list package directory):
    $ make clean 
    $ make

2. How to install the debug version?
   run from linux command line (from list package directory):
    $ make clean
    $ make VER=debug

RUN
------------------------------
1. release version:
    run from 'bin' directory: ./thread_pool.out
2. debug version:
    run from 'bin' directory: ./thread_pool_debug.out
