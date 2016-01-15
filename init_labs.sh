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

# Delete the old origin, this will need to be moved to the students
# local repo
git remote rm origin

# Add an upstream repo for bug fixes that need to be pushed to students
git remote add upstream git@github.com:CprE308/labs.git

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

#todo:
#add a new origin endpoint that is to the students submission repo.  This will need to be requested from the student, or maybe it could work by using `uname`.

#Idea:
git remote add origin git@github.com:CprE308/student-`whoami`.git

