#!/bin/bash
# 
# This shell script will set up your git repositories to access the labs 
#
# Usage:
#     ./init_labs.sh
#
# After running this script you can pull a lab (e.g. lab 1) by typing
#     git pull lab1 master
# 
# If the above command fails then the lab has not been released to be 
# visible yet.


# Test SSH Keys to GitHub to make sure the student has access
ssh -T git@github.com 2>&1 >/dev/null | grep -c "successfully authenticated" > /dev/null
rc=$?;
if [[ $rc != 0 ]]; then
	echo -- SSH Keys not set up! --
	echo Please create an SSH key pair and add them to your
	echo GitHub account.  If you have questions ask your TA
	exit -1;
fi

# Get the students NETID
read -p "Enter your NETID: " netid

# Clone the root labs repo
git clone git@github.com:CprE308/labs.git
rc=$?;
if [[ $rc != 0 ]]; then
	echo -- You do not have access to the labs repo --
	echo Please ask your TA for assistance getting your
	echo GitHub username added to the class team
	exit -1;
fi

#enter the new repo
cd labs

# Delete the old origin, this will need to be moved to the students
# local repo
git remote rm origin

# Add an upstream repo for bug fixes that need to be pushed to students
git remote add upstream git@github.com:CprE308/labs.git

# Change origin to the students submission repositiory
git remote add origin git@github.com:CprE308/labs-$netid.git

# Pull any local changes the student has made to their repo
git pull origin master

# push to set up default end point and make sure everything is in sync
git push --set-upstream origin master
rc=$?;
if [[ $rc != 0 ]]; then
	echo -- Failed to connect to your submission lab repo --
	echo Please ask your TA for assistance to fix this problem.
	echo Please delete the labs directory using 'rm -rf labs'
	echo before re-running this script.
	exit -1;
fi

# Add remotes for each lab of the semeseter
git remote add lab1 git@github.com:CprE308/lab-01.git
git remote add lab2 git@github.com:CprE308/lab-02.git
git remote add lab3 git@github.com:CprE308/lab-03.git
git remote add lab4 git@github.com:CprE308/lab-04.git
git remote add lab5 git@github.com:CprE308/lab-05.git
git remote add lab6 git@github.com:CprE308/lab-06.git
git remote add lab7 git@github.com:CprE308/lab-07.git
git remote add lab8 git@github.com:CprE308/lab-08.git
git remote add lab9 git@github.com:CprE308/lab-09.git
git remote add lab10 git@github.com:CprE308/lab-10.git
git remote add lab11 git@github.com:CprE308/lab-11.git
git remote add lab12 git@github.com:CprE308/lab-12.git
git remote add final git@github.com:CprE308/final-project.git

# print out all remotes
git remote -v

# print user message
echo You have successfully set-up your labs folder.  To continue
echo please pull down the master branch of lab1 by running the 
echo following command:
echo     git pull lab1 master

