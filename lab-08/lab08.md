---
title: 'Scheduling'
---

# Scheduling
This lab will be exploring the design of several scheduling algorithms and compairing their response times and other features.  First, **please** take a look around the provided code in the project folder.  Nearly all of this has been finshed for you; however, it is good to look through it and understand how the different parts work.

## Compiling the algorithm
One of the scheduling algorithms has already been finished for you as an example.  `cd` into the `scheduler\fcfs\` directory and run `make`.  next `cd` back to the `project` directory and run `make`.  To test it, run the following command:

~~~bash
$ ./sched_test scheduler/fcfs/sched-fcfs.mod
~~~

When this finishes it will create two files: one called `fcfs.log` which show information about each task along with the overall averages, and one called `fcfs.json` which you can copy the contents of into [http://www.wavedrom.com/editor.html](http://www.wavedrom.com/editory) to see a Gantt chart of the scheduler.

## Tasks for this lab
Your tasks for this lab are to implement the Shortest Remaining Task Next, Round Robin, and Priority Round Robin schedulers.  You can test multiple schedulers using the `sched_test` program by suplying each as a command line argument.  The expected outputs from each scheduler are given in the `answers` directory to compair with.  _note, I can't garuntee that the answers are correct, I finished them up at 5AM and have not verfied them_.  Comment in your lab report the outputs of each algorithm and how they compare with each other.  Is it what you would expect?  Which do you consider the best?

## Going futher
After you have finished the scheduling algorithm you should now play around with the numbers to see how it effects the response times.  In the `init_test_framework` function in `sched_test.c` you can chage the rate which new processes enter the system by changing the modulo value on the `start_time +=` line, and the length of each process by chaging the `length =` line.  Take care to make sure the length is never `< 1` as this will cause problems with the test program.  Also try changing the quantum of round robin algorithm to see how that effects the system.  Comment on what you do and your observations on the effects in your lab report.

# Extra Credit
For extra credit you can implement your own scheduling algorithm.  Feel free to get as creative as you would like on this and follow the given examples to get started.  One option is priority round robin with different quantum for each priority level.

# License
This lab write up and all accompany materials are distributed under the MIT License.  For more information, read the accompaning LICENSE file distributed with the source code.




