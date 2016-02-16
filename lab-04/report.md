# Threading using PThreads

## Creating Threads in a Program

Some changes had to be made to pthread_test to get it to compile
with full compiler warnings turned on.

Per POSIX, pthread_create takes a void * ( *fn) (void *) for the
start_routine parameter. This is not the same as void ( *fn) (void).
PThreads requires a function which returns a void pointer and takes a void
pointer as its only argument. Second, it is undefined behavior in C11 (and even previous C standards) to
cast void * to a function pointer, or a function pointer to void *.

The program was modified for the thread initialization
functions to have the proper signature for pthread_create and
eliminate the need for casting to void * and relying upon
undefined behavior.

### What is one expected output of running this program?

One expected output is
```
I am thread 1
I am thread 2
I am thread 0
```

### What is the actual output of the program?
```
jake at gilliam in ~/308/lab-04 on master*
$ gcc -Wall -Wextra -Wpedantic -Werror -std=gnu11 pthread_test.c -pthread -o pthread_test
jake at gilliam in ~/308/lab-04 on master*
$ ./pthread_test
I am thread 0
jake at gilliam in ~/308/lab-04 on master*
$ gcc -Wall -Wextra -Wpedantic -Werror -std=gnu11 pthread_test.c -pthread -o pthread_test
jake at gilliam in ~/308/lab-04 on master*
$ ./pthread_test
I am thread 1
I am thread 0
I am thread 2
jake at gilliam in ~/308/lab-04 on master*
$ ./pthread_test
I am thread 0
jake at gilliam in ~/308/lab-04 on master*
$ ./pthread_test
I am thread 0
```

Observations: If thread 0 terminates before threads 1 and 2 print, they will not
get a chance to print.

## Joins

### What is the output of the program?

```
jake at gilliam in ~/308/lab-04 on master*
$ gcc -Wall -Wextra -Wpedantic -Werror -std=gnu11 pthread_test.c -pthread -o pthread_test
jake at gilliam in ~/308/lab-04 on master*
$ ./pthread_test
I am thread 1
I am thread 2
I am thread 0
jake at gilliam in ~/308/lab-04 on master*
$ ./pthread_test
I am thread 1
I am thread 2
I am thread 0
jake at gilliam in ~/308/lab-04 on master*
$ ./pthread_test
I am thread 1
I am thread 2
I am thread 0
jake at gilliam in ~/308/lab-04 on master*
$ ./pthread_test
I am thread 1
I am thread 2
I am thread 0
```

### Does it match with one of the expected outputs of the program?

Yes. The joins ensure that both created threads will run before the main
thread returns. It would be possible for Thread 2 to print before Thread 1, but
that never ended up happening to me.

## NQueens

It was a relatively simple task to expand the provided nqueens to include multi-threading.
The creation of the rows array and initial call of queens_helper into the
thread function. The for loop then invokes a thread for each column, and
afterwards collects all threads with join() and sums their solutions_found.

Scheduling for multiple threads is accomplished with a simple algorithm:
Each thread grabs the next available column to calculate. Thread 0 grabs
Column 0, then as threads start (or finish), they grab the next column
(Column 1 and so on), incrementing the next_column variable for
whichever thread comes next. Columns are then calculated
in order, from 0 to num_queens - 1.

A mutex is used to protect the next_column variable.

### Notes on Running

The -t flag takes a mandatory argument: the number of threads in which to run.

When running multithreaded, the -v flag provides very useful debug
output for seeing how the scheduling algorithm causes threads to pick up
individual columns

### Performance Notes: N Queens N Threads
Running for 14 queens, the results below can be observed:
```
jake at gilliam in ~/308/lab-04/nqueens on master*
$ ./nqueens -n 14
# Queens = 14, Threaded = FALSE, Verbose = FALSE, Display = FALSE
Real Time: 0:21.06, User Time: 21.06, System Time: 0.00, CPU Usage: 99%
Solution Count = 365596
jake at gilliam in ~/308/lab-04/nqueens on master*
$ ./nqueens -n 14 -t
# Queens = 14, Threaded = TRUE, Verbose = FALSE, Display = FALSE
Thread 00: Real Time: 0:01.98, User Time: 1.17, System Time: 0.00, CPU Usage: 59%
Thread 02: Real Time: 0:02.08, User Time: 1.49, System Time: 0.00, CPU Usage: 71%
Thread 03: Real Time: 0:02.20, User Time: 1.54, System Time: 0.00, CPU Usage: 70%
Thread 13: Real Time: 0:02.32, User Time: 1.19, System Time: 0.00, CPU Usage: 51%
Thread 07: Real Time: 0:02.34, User Time: 1.67, System Time: 0.00, CPU Usage: 71%
Thread 01: Real Time: 0:02.46, User Time: 1.36, System Time: 0.00, CPU Usage: 55%
Thread 06: Real Time: 0:02.47, User Time: 1.67, System Time: 0.00, CPU Usage: 67%
Thread 11: Real Time: 0:02.52, User Time: 1.49, System Time: 0.00, CPU Usage: 59%
Thread 12: Real Time: 0:02.60, User Time: 1.36, System Time: 0.00, CPU Usage: 52%
Thread 09: Real Time: 0:02.60, User Time: 1.61, System Time: 0.00, CPU Usage: 61%
Thread 10: Real Time: 0:02.65, User Time: 1.54, System Time: 0.00, CPU Usage: 57%
Thread 04: Real Time: 0:02.71, User Time: 1.61, System Time: 0.00, CPU Usage: 59%
Thread 08: Real Time: 0:02.73, User Time: 1.64, System Time: 0.00, CPU Usage: 60%
Thread 05: Real Time: 0:02.78, User Time: 1.64, System Time: 0.00, CPU Usage: 58%
Real Time: 0:02.78, User Time: 21.08, System Time: 0.00, CPU Usage: 756%
Solution Count = 365596
```

Note that despite the tremendous difference in Real Time, the User Times are very similar.
The 0.02 second increase in User Time can be explained by thread overhead - the time needed
to set up and tear down the threads.

For real time, there was no number of threads, where queens = threads, at which the threaded solution ran
slower, given the time was measurable. The real time was not measurable until N = 10, at
which point both the threaded and unthreaded times were equal to 0.01s. At N = 11, the 
non-threaded solution was notably slower, at 0.08s vs 0.01s.

The user time of the threaded solution was often several times higher than the non-threaded times for
very low (0.01s) real-time runs. This is probably due to rounding errors, where ~0.005 rounds 
up to 0.01 for non-threaded, but it rounds up to 0.01 and then sums 8 times for the threaded run.
In either case, the difference is trivial and disappears once run-times become measurable on the scale
of seconds.

The number of threads is not linear with respect to the number of queens because the number of 
simultaneously running threads is capped by the number of hardware threads supported by the system.
In the case of my system (two quad-core Xeon processors), 8 threads can run
simultaneously, giving a result of close to 800% CPU utilization. Adding any more threads
will just cause threads to have to share user time, resulting in no gain in real-time performance.
In fact, this will result in minute losses due to context switches between running threads.

### Performance Notes: N Queens T Threads
When the number of threads can differ from the number of queens, 
better experimentation can be performed.

I expected the fastest run time for any given number of queens to occur when the number
of threads was set equal to the number of physical threads available (8), however this
was not exactly the case. For value of N in the range of 13-15, it actually ran quickest with 
the number of threads equal to the number of queens. The next most efficient run time
was with the number of threads equal to physical threads. Values in between were notably slower,
and incurred notably worse CPU usage.

To me, this indicates inefficiency with my scheduling/work assignment algorithm. Perhaps it
is an artifact of the order in which threads finish, since there seems to be a significant delay
between the first threads finishing and the last. With less than 8 threads running,
Real Time is being wasted. When the last few (2 or 3) threads take a few seconds
longer to finish, a large amount of real time is wasted. The solution would be
optimising my scheduler to assign the longest task before the second-longest task, etc. This
way, the threads would finish at the same time.

#### Alternative scheduler algorithm: Decremental

I adjusted the scheduler algorithm to use a decremental, rather than incremental, approach.
The idea was to run the longest threads first, so that the shorter-running threads would 
better "fill in" the end of the process, letting the threads finish closer together. However, the
drastic difference between columns still prevents the threads from finishing near the same time.

The decremental scheduler does have better performance than the incremental scheduler with thread
counts of 2 or 4. This is because stepping by 2 is a much more gradual time difference, allowing
better granularity for the "filling in" to occur. However, such low thread counts have awful
performance compared to higher thread counts, so this is not of much use.
