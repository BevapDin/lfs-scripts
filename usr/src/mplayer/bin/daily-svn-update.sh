#! /bin/bash

export PATH="$HOME/bin:$PATH"

working_dir="/usr/src/mplayer/mplayer@svn"

cd "$working_dir" || exit $?

svn up || exit $?

make clean
if ! cfgc --no-install --quiet ; then
	ret="$?"
	cat make.{1,2}.log
	exit "$ret"
fi

cmp-mplayer-versions.sh --batch

for d in mplayer mencoder ; do
	/bin/cp --remove-destination --backup=numbered "/usr/bin/$d" "$HOME/$d.bak"
	/bin/cp --remove-destination "$d" "/usr/bin/$d"
done

