# Lab 8: Scheduler Algorithms

## FCFS
The FCFS waveform (fcfs-wave.json) can be viewed with Wavedrom. This algorithm
has an apparently random distribution of tasks. It disregards priority, but does
allow each program a chance to run before another task runs twice. This, however,
does not provide any real-time guarantee, since a task is permitted to run for
and indefinite amount of time before it yields.

Starvation is not possible in FCFS, but only by a soft guarantee.

## RR
Round robin (rr-wave.json) begins looking quite similar to FCFS. This
is because there are very few active tasks at the beginning. However, once
the workload increases, the round-robin nature of the algorithm can be
seen. No task runs for more than a single quantum, and each task gets a chance
to run before any other task runs. The top row makes it quite obvious that
each task only runs for a single quantum, and the cyclic pattern can
be seen across individual tasks. It can also be observed that no task
goes a particurarly long time in the "READY" state. This is also
a notable difference from FCFS.

RR still does not respect priority, but starvation is not possible.

## PRR
Priority round-robin was implemented using four separate circular queues. When
a task is added to a queue, it is added to the appropriate queue based on priority.
When the periodic task is called, the algorithm will first look for a priority
3 task to run. If no priority 3 task is ready to run, it will then look for a 
priority 2 task. It then continues with 1 and then 0.

Priority round-robin (prr-wave.json) looks similar to Round Robin, but clearly
respects priority. With PRR, there are long segments
of a single color (priority) running before it drops to a different priority.
PRR also exhibits behavior where a single task will run for more than
one quantum even when other tasks are waiting. This happens when there is
only one task of the highest priority: it will not yield to a lower priority task.

Note that blue and white tasks (priority 1 and 0, respectively) do not execute
until very near the end.

Tasks can suffer starvation, but only if higher-priority tasks exist.

## SRTN
Shortest remaining time next utilizes the scheduler_data field of tasks
to store two pieces of information: the guess and the runtime as of the
most recent time the task was added to the ready queue. When a task is
added to the ready queue, these fields are created if they do not exist. Otherwise,
they are updated based on the previous values.

Whenever a task is added to or removed from the queue, the algorithm examines
the tasks to find the one with the best estimate of SRTN, which is then set as
the new head and returned.

SRTN (srtn-wave.json) appears quite similar to the Priority Round Robin. This is
because the higher priority tasks declare a shorter initial time remaining. This
causes them to run before the lower priority tasks. As long as a high priority task
adheres to its declared time, it will continue to run before the low priority tasks.
This causes the starvation of lower priority tasks seen with SRTN.

SRTN also sees a pattern emerge with a series of similar-length tasks running in
succession. This is a good indicator that the prediction is working fairly well.

Tasks can suffer starvation, but only if higher priority tasks are created and those
high-priority tasks consistently run with short execution times.

# Conclusion
FCFS is a trivial yet inefficient algorithm. It fails to respect priority, and only
offers a soft guarantee of time constraint before a task gets to run. RR upgrades
this to a hard real-time guarantee: Each task will only have to wait an amount of
time equal to the number of tasks * the quantum before it gets a chance to run. RR 
also retains the guarantee that no task will run twice before allowing another
a chance to run.

Priority Round Robin and SRTN offer privilege based on priority. With PRR, a
higher priority task can run twice before a lower task. This allows starvation
to occur of lower-priority tasks, and can also cause a task to run for
more than one quantum if it is the only task of that priority or higher.

SRTN only initially privileges tasks based on priority. The only way for
a task to retain its privilege is to consistently run in short bursts. If a
high-priority task consistently runs for long periods of time, its SRT estimate
will increase and it will fall to a lower effective priority.
