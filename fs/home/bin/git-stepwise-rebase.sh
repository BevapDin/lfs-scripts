#! /bin/bash

while true ; do
	commit="$(git-list-next-upstream-merge.sh)"
	if [ -z "$commit" ] ; then
		break
	fi
	git rebase "$commit" || exit $?
done

exit 0
