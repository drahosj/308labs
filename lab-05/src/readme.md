# Print Server
## Compilation
- create the printer/drivers directory
- cd printer && make && cd ..
- make

## Launching the test
- sh start_drivers.sh
- sh test.sh | ./printserver -l test.log

## Test Case
The provided test.sh sends 48 jobs to the print server.
These are relatively evenly divided between color and black/white.

No additional files are used beyond the provided samplec.ps.

## Test Results
To verify test success, run
~~~
find . -name 'output*.pdf' | wc -l
~~~
48 files should be created.

When EXIT is received, the main thread will print a waiting message and
set the kill flag. The program will not terminate until all pending jobs
have been handled and all threads have terminated.

The test.log file will detail start and stop times of each job. The duration
is reported to one-second resolution.

NB: The one second resolution is too broad for the given file with virt-printer. All
durations report 0 seconds.
