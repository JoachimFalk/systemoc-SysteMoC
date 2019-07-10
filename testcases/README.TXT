Execute the following command to see all remote branches on origin.

...]$ git branch -a

Then select a remote branch to checkout, e.g.,
origin/BuildSystem--mainline--0.4, and get it with:

...]$ git checkout -b <branch> origin/<branch>
