#! /bin/sh

# This "updates" your remote master branch (origin/master),
# actually it just overrides the branch with the current master from upstream.
# upstream == the original repository where your branch is forked from.
# origin == your personal repository (you obviously need write access to it).
# Danger, Will Robinson! This will override your remote master branch. Local branches are not affected.

# No parameters for you!
if [ "$#" != 0 ] ; then
	echo "This script does not use any paramters." 1>&2
	exit 1
fi

# Get the current status from upstream
git fetch upstream

# Now the origin/master (my repository) to upstream/master
git push origin upstream/master:master
