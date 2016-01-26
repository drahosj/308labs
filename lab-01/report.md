Lab 1 Report
------------

I am already very experienced using the tools discussed
in the early parts of this lab, so most of it was a review.

## Tools Review

## Git

My git-fu is extremely strong, so nothing in this lab was
new or particularly surprising.
That said, setup_labs.sh contains some
of the most interesting (ab)use of tags and remotes
I have ever seen.

### GDB

Breakpoints, stepping, print, resuming, quitting, etc. are all
basic features of GDB. Nothing particularly unorthodox occurs
here.

When debugging the segfault, there are some GDB commands, such as frame
and backtrace, that I am not familiar with. I tend to use "where" and then
breakpoints on a re-run, but using frame manipulation seems like a much
faster and more efficient way to handle it, especially if segfaults
are unpredictable or occur deep into a debug session.

### Valgrind

It has been a while since I last used Valgrind, so I was not entirely
familiar with the output, and also have forgotten the syntax for some 
of the advanced leak checking and other functionality.

## rand_string.c teardown

It was a pretty easy fix: the first loop used <= instead of <. 
This caused one extra to be written beyond the end of the 
rand_array, which (on my machine) clobbered the pointer to the
string. Dereference the pointer, and bang. Segfault.

Finding this was pretty trivial. Run under GDB, cause segfault,
determine where it occurred. Try reading from each variable: all read
properly except for string. It's pointer value was also quite weird.

Restart the program, and step through testing string. It first
became clobbered after the initial loop. A quick glance over the loop
and it is easy to see where the bug is.


## Conclusion

It was relaxing to use these tools in a situation that isn't "I've been working
on this for eleven hours and it's 3 AM please just find the problem so I can fix
it and sleep for at least one night this week and why is the stack pointer being
initialized into nonexistent memory I hate embedded systems why are hard faults so hard
to debug?".

Normally that's the sort of attitude I have when I'm using GDB. This was a refreshing break. with a quick solution.
