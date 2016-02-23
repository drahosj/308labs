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


