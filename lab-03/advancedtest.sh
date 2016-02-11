#! /bin/cash

# Advanced test
# This demonstrates the advanced features of CASH

echo What type is echo?
type echo
echo Looks like it is a builtin. Congratulations on loading the plugins!

# In-line comments
echo Hello, world! # This part is a comment

# CASH supports variables

export FOO= bar
echo Foo is $FOO # See?

echo
echo You can Ctrl-C out of this at any point. CASH respects Ctrl-C when
echo running non-interactively.

sleep 10

echo Now check out CASHs backgrounding
sleep 4

echo
echo sleep 8 <backgrounded>
sleep 8 &

echo
echo sleep 7 <backgrounded>
sleep 7 &

echo
echo sleep 6 <backgrounded>
sleep 6 &

echo
echo sleep 5 <backgrounded>
sleep 5 &

echo
echo sleep 1 <backgrounded>
sleep 1 &

echo Sleep for a little while to reap the last sleep

echo
echo sleep 2
sleep 2

echo
echo That last sleep should have just been reaped.
echo Now we will sleep a bit longer to reap those first few sleeps

echo

echo sleep 10
sleep 10

echo
echo Good, all of the children should have been reaped.

sleep 4

echo
echo Time to show off the plugin features
sleep 5
echo First, we delete some critical plugins:
echo

sleep 2
echo rm plugins/echocd.so
rm plugins/echocd.so
sleep 2

echo rm plugins/exit.so
rm plugins/exit.so
sleep 2

echo
echo Now we launch another instance of CASH. See you soon!

sleep 8

./cash advancedtest_stage2.sh

echo So what did you think of that? Pretty neat!
sleep 2
exit
