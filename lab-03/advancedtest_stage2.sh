#! /bin/cash

echo Hello, welcome to the second CASH instance running.
sleep 5

echo
echo This one is missing a bunch of critical builtins, because
echo the plugins were not loaded (did you see the warnings?)
sleep 5

echo
echo We cannot cd
sleep 2

echo PWD:
pwd
sleep 2

echo
echo cd plugins
cd plugins
pwd
echo
sleep 6

echo cd ..
cd ..
pwd
echo
sleep 4

echo
echo We cannot even exit!

sleep 2
echo
echo exit
exit

sleep 10
echo
echo But how can we echo?
sleep 5

echo
echo type echo
type echo

sleep 3
echo
echo Because echo is also a binary provided by the system, not a builtin!

sleep 3
echo Time to do something about that.

sleep 4
echo
echo make
make

sleep 4
echo
echo Now the plugins are rebuilt, time to load them.
sleep 8

echo
echo Fortunately we still have pluginmanager.so loaded.
sleep 2
echo That means we have load and loadall

sleep 5
echo
echo loadall
loadall
sleep 5

echo
echo Now that our plugins are loaded, we should have our builtins back!

sleep 2
echo
echo PWD:
pwd
sleep 4

echo
echo cd plugins
cd plugins
pwd
sleep 4

echo cd ..
cd ..
pwd
sleep 4

echo
echo echo should be a builtin again

sleep 5
echo
echo type echo
type echo

sleep 8
echo
echo And now we can exit. Bye!

sleep 5
echo exit
echo
exit
