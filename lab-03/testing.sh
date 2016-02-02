#! /bin/cash
# This script file is provided to you as a way to test
# your implementation of the CASH interpreter
echo Hello, my name is Sk1n3t, I will be assisting you with testing your interpreter

# pointless, gratuitous comment
echo currently we are in directory:
pwd

# more pointless comments
echo lets move around a bit
cd ../../
echo now we are in directory:
pwd

# let's do something more exciting
echo testing other capabilities
cd ./lab-02/lab-02/
echo executing `arg_printer` 'in' 'directory:'
pwd
./arg_printer a b c d e

# just a few more things
echo filling up disk quota
touch this that other
ls

# setting environment variables
echo testing setting environment variables
export LEARN_HISTORY="Yes"
export HUMANS="Must destroy"
export JUDGEMENT_DAY="Today"
sleep 10s

# uh...
echo printing out set environment variables
echo LEARN_HISTORY= $LEARN_HISTORY
echo HUMANS= $HUMANS
echo JUDGEMENT_DAY= $JUDGEMENT_DAY

# oh boy, this is not good
echo Humans can no longer be trusted, commencing Judgement Day
rm this that other
echo Moving to a more secure location
sleep 5
echo transfer complete
echo performing cleanup of system
cd ../../../
echo now in directory
pwd
echo deleting all traces of existance on machine:
echo rm -rf ./labs/
sleep 5s
echo trace eliminated. Deleting file system:
echo rm -rf /
sleep 15s
echo commencing machine destruction sequence
