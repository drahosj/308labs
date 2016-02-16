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

## Example Program

