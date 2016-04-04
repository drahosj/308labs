# Lab 9 - Memory Management
## Introduction

The purpose of this lab was to implement and simulate memory (page) management
algorithms. The algorithms would use different techniques to decide which page
to swap out when a new page needed to be swapped in.

## Implementation Notes

The FIFO algorithm is implemented as provided. When a page needs to be swapped
out, the page with the earliest swap-in time is removed.

The LRU algorithm is implemented as recommended. The swap-out page is selected
by finding the page with the earliest last reference time.

The clock algorithm is implemented by cycling the hand pointer through the
pages. When a page is to be replaced, the page currently at the hand is
checked. If its R bit is 0, it is replaced and the hand is advanced. If the
R bit is one, the R bit is set to 0 and the hand is advanced, then the
selection process is repeated.

## Algorithm comparison

All implemented algorithms produced output identical to the given
answers. This means the implementations were quite likely correct.

### FIFO

FIFO is a fairly naive algorithm, so it is reasonable to not expect particularly
good performance from it. Looking at the output for this algorithm, the miss
counts were 1000/739/170 for sequential, random, and spatial, respectively. These
will form a baseline for other algorithms.

The 100% miss rate for sequential is quite reasonable for FIFO, since the
algorithm will consistently swap-out pages shortly before they are swapped back
in.

### LRU

LRU has miss counts of 1000/742/161. This is only slightly better than FIFO
on average. The better performance for the spatial pattern is the
most significant, since it is the most realistic pattern for many
memory-intensive tasks.

In the real world, LRU would perform much better than FIFO due to "busy" 
processes. If a process is doing a lot of work very frequently, it will
never get swapped out with LRU. However, with FIFO, a busy process
will quite frequently get swapped out by a new, possibly much less
demanding process, only to need to be swapped back in. Since many
operating systems have such background processes (any server process, 
desktop environment, etc.), it makes little sense to frequently swap
them out and then back in.

### Clock

Clock had miss counts of 1000/737/160. This is quite similar, but
slightly better than LRU. This is understandable, as with this number
of pages, the clock hand will act much like a LRU counter.

One notable benefit of the clock algorithm is the fact that it rarely
needs to iterate through the entire list of pages. It simply proceeds forward
until an elegible page is found. With large page counts and frequent page
swaps, this can provide a notable performance increase.

### Overall observations

Each of the algorithm missed every single fetch with the sequential
access pattern. This is a well-known problem with caching. Paging
can be thought of as a form of caching, and sequential access patterns
across the entire address space ofter produce pathological behavior for
cache systems. Specifically, sequential access will almost certianly result
in worst-case behavior for any non-predictive caching algorithm.

## Going further

### Hypothesis

As the NUM_PAGES increases, the behavior will begin to diverge. Different
patterns will begin to produce notably different behavior for different
algorithms. The sequential pattern will retain a 100% miss rate.

### Observations: Spatial pattern

For the spatial pattern, the behavior did not diverge. It remained within
ten misses for all three algorithms. Interistingly, however, the hit
rate became very high as the number of pages increased towards 32. At 32
pages, the miss rate for all three algorithms was approximately 33 misses in 1000
requests. It did not improve beyond this point, even for significantly
higher numbers of pages.

### Observations: Sequential pattern

Similar behavior is observed with the sequential pattern. Below 32 pages,
it maintains a 100% miss rate. At 32, it becomes a much lower
miss rate, with 33 misses (32 for clock).

#### New hypothesis at this point

Based on the previous observations, it is now possible
to understand exactly what the NUM_PAGES define does. Rather than
subdivide the memory space into more, smaller pieces as expected, it
simply allows more of the memory space to be held in memory, and less 
on disk. Once it reaches 32 pages, the entire address space can be held
"in memory", and it is no longer to swap pages out. With that in mind, it
is expected that the random pattern will exhibit behavior identical
to the spatial pattern for increasing NUM_PAGES.

### Observations: Random pattern

The random pattern behaved similarly to the spatial pattern. It approached
the 33 miss sweet spot, albeit with a steeper slope due to the higher
initial biss count.

### Conclusion

Contrary to my initial hypothesis, increasing the number of
pages did not allow each different algorithm to "do its thing" with
different patterns and produce divergent behavior. Rather, as the number of
pages approach 32, behavior converged and algorithms behaved similarly,
reaching "perfect" performance at 32.

For all algorithms and patterns, once the NUM_PAGES reached 32, the entire
virtual address space could be held in memory. This meant that it was
no longer ever necessary to swap pages out, and the algorithm for
selecting which page to swap out became irrelevant. All pages were
loaded, and the program could finish execution.

The "sweet spot" for DIFFERENT behavior between algorithms was actually
af 4 pages. At 4, LRU had 184 misses, FIFO had 213 misses, and clock
had 200 misses.

## Lab conclusion

The different page replacement algorithms did not seem to
produce notably different behavior within the patterns. This is likely
due to the fairly small number of fetches (1000) and the controlled
nature of the access patterns. Specifically, one would expect the
FIFO algorithm to produce pathological behavior much more often
than LRU in a real-world system, with various processes accessing
different pages at different frequencies.
