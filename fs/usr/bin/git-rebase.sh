#! /bin/bash

if [ "$#" = 0 ] ; then
	git fetch upstream
	options=""
else
#	options="--ignore-whitespace -Xignore-space-change -Xignore-all-space -Xignore-space-at-eol"
	options=""
fi

git rebase $options upstream/master "$@"
