#! /bin/bash

# Update the mplayer package from current svn, completely automated.
# This is intended to be run from a cron job. Running it from the console is also possible.

export PATH="$HOME/bin:$PATH"

# Where the svn checkout is.
SVN_CHECKOUT_DIR="/usr/src/mplayer/mplayer@svn"

if ! -e [ "$SVN_CHECKOUT_DIR" ] ; then
	svn checkout "svn://svn.mplayerhq.hu/mplayer/trunk" "$SVN_CHECKOUT_DIR" || exit $?
fi

cd "$SVN_CHECKOUT_DIR" || exit $?

# Can be set to prevent this script from running, useful when this script is called
# from a cron job. It simulates success, so cron does not complain.
if [ -e ".no-daily-svn-update" ] ; then
	exit 0
fi

# Update from svn server
svn up || exit $?

make clean
if ! cfgc --no-install --quiet ; then
	# If make (or configure) failed, print the log files and the environment
	echo "Output of make:"
	cat make.{1,2}.log
	echo "Environment:"
	env
	exit 1
fi

# Just for the fun: compare the new and the current binary.
cmp-mplayer-versions.sh --batch

# Install binary, but make a backup, using explicit /bin/cp to avoid the wrappers for the package user.
for d in mplayer mencoder ; do
	/bin/cp --remove-destination --backup=numbered "/usr/bin/$d" "$HOME/$d.bak"
	/bin/cp --remove-destination "$d" "/usr/bin/$d"
done
