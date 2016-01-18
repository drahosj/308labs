Lab Subissions
==============
This is your main lab submission repository.  All of your labs for this semester will be submitted using git by pushing this repository to GitHub.  This repository should have been created using the `setup_labs.sh` script available on the class Blackboard.  That script sets up the different remote repositories that are need to successfully complete these labs.

Getting a Lab
-------------
Each week a new lab will be made available via GitHub.  You will need to issue a `git pull` command to retrieve that new lab.  Each pull will need to include which lab you want to pull and what branch to pull from.  All the labs are on the `master` branch.  To pull lab 1 issue the following commands:

~~~bash
$ git pull lab1 master
~~~

Replace lab1 with lab2 to pull lab 2 and etcetera for future labs.

Saving Your Work
----------------
As you are working on your labs it is a good idea to push your updates reguarly to GitHub.  By doing this if something happens to your local copy you have a backup to look back on.  Make sure to give each commit a detailed message of exactly what channged in that commit.  To save your work you will first need to see what files changed by running `git status`.  Files that show up under `Untracked files` are files that have been created but are not staged for commit.  To stage these files for commit use `git add filename`.  Once you have all the changed files that you want to save saged run `git status` again to double check.  Then you can push by typing `get push`.

Submitting You Labs
-------------------
When you have finished a lab you will need to submit it for gradding.  First, you must tell the TA exactly what point in the repository history you were done with a lab.  To do this you will create a tag for that lab.  Lab 1 submission should be tagged `lab1`, and lab 2 should be tagged `lab2`.  To create a tag for Lab 1 first follow the instructions above for Saving Your Work.  Then enter the following command:

~~~bash
$ git tag lab1
~~~
You can check the tag by typing `git show lab1`.  This show the commit that will you will be gradded against for that lab, the author that made the tag, and the date the tag was created.  This date will be used to tell if the lab was submitted on time or not, so please make sure that you create the tag by the due date of the lab.

After creating the tag you still need to sync that tag with the remote repository so the TAs can grade.  To do this type the following command:

~~~bash
$ git push origin --tags
~~~

