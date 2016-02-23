# Memory management with threads

## Threaded Count

### What is the expected output?
The expected output is either 2 million, since each thread will increment shared 1 million times.

### What is the actual output?
The actual output, in my case, was 1007409.
~~~
jake at gilliam in ~/308/lab-05 on master*
$ gcc -o threaded_count threaded_count.c -Wall -Werror -Wextra -Wpedantic -std=gnu11 -Wno-unused-parameter -Wno-return-type -lpthread
jake at gilliam in ~/308/lab-05 on master*
$ ./threaded_count
After both threads are done executing, `shared` = 1007409

### What caused the discrepancy between the expected and calculated values?

This is caused because the unary ++ operator, despite its appearance, is not an atomic operation.
It is a complete read-modify-write three-part operation. This can cause one thread to read it, modify it,
and have the other thread read-modify-write it before writing it back. In my case, it seems that this
happened the majority of the time, since it is barely over 1 million.

## Mutex locks

### Did this fix the issue with the original code?

Placing a mutex lock and unlock around the increment statement solved the problem. The desired output
of 2 million is now achieved.

## Waiting on the conditional variable to change
### What is the mimimum number of conditions needed for the example to work as intended?
The example can be fixed with one condition.
### What would those conditions be, and which thread should wait on that condition?
The condition can be named 'produced'. A consumer thread will obtain the mutex, then wait on
the produced condition if there is nothing available. If a second consumer thread reaches this same point,
only one will be unblocked by the pthread_cond_signal, and will then finish running.

## Print Server

### Design Philosophy

Two static functions were created to handle queue operations (put and get).
The queue is protected by a simple mutex lock. The put operation increments the
size semaphore.

The producer thread, when applicable, will create a job and post it to the queue.
The consumer threads do a delicate yet efficient dance to check the state of the
job queues and a kill flag. When commanded to exit, the main thread will
set the kill flag.

The consumer threads will handle any pending jobs before responding to the kill
flag. This is accomplished by doing a sem_trywait on the job queue to check
for pending jobs. If the sem_trywait indicates a pending job, an internal flag will
be set to defer that job until the kill flag mutex can be released.
If there is no pending job, the kill flag is then checked. If the kill
flag is set at this point, the thread will return.

If the sem_trywait defferred a job to handle, that job will now be handled. The 
busy loop will then repeat. If the busy loop indicates no immediately waiting job
and no kill flag, the thread will then enter a one-second wait period on the
job queue semaphore. If a job appears in this time, it will be handled immediately.
Otherwise, after the timeout, the busy loop will be repeated to check for 
the kill flag.


### Problems Identified and Solved

#### Jobs not finished prior to exit

The check for immediately pending jobs and the kill flag must be atomic, otherwise
the producer thread can create a job and set the kill flag at the "same time". This
will cause a race condition and all sorts of weirdness, resulting in no clear
way to tell whether or not a job will be completed if it created near the
issuance of the EXIT command.

#### Excessive time spent holding kill_flag lock.

In a previous implementation, the handle_job routine was performed while holding
a mutex for the kill_flag lock. This prevented other threads from checking
for the kill flag, and effectively allowed only one thread to process at once.

With the current implementation, the only actions that occur while the kill_flag
lock is held is the checking of the kill flag and the semaphore check for an immediately
pending job. If a job is found, its handling is deferred until the mutex is released.


### Testing

See src/readme.md
