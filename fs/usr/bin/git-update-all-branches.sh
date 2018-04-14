#! /bin/bash

if ! [ -z "$(git status -s)" ] ; then
	echo -n "There are changes in this checkout. Continue anyway? (Answer with uppercase yes): "
	read a || exit $?
	if [ "$a" != 'YES' ] ; then
		exit 1
	fi
fi

# Get all remote updates. One can skip this, but you'll probably want to check against the
# most recent branch information, so get it. Errors are ignored, the user might have already
# updated their repository and why fail here?
git fetch --prune upstream
git fetch --prune origin

get_current_branch() {
	git branch | sed 's#^\* ##;t;d'
}

current_branch="$(get_current_branch)"

do_skip() {
  local e match="$1"
  shift
  for e ; do [[ "$e" == "$match" ]] && return 0; done
  return 1
}

do_branch() {
	local branch="$1"
	git checkout "$branch" || return $?
	git rebase upstream/master && return 0
	while true ; do
		read -p "[C]ontinue, [s]kip, [a]bort rebase, open with [g]eany, [e]xit?" ans || return $?
		case "$ans" in
			g)
				git status --short | sed 's#^UU ##;t;d' | xargs -r geany
				;;
			c|C)
				git rebase --continue && return 0
				;;
			s)
				git rebase --skip && return 0
				;;
			a)
				git rebase --abort && return 0
				;;
			e)
				return 1
				;;
		esac
	done
}

for branch in $(git branch | sed -n 's#^\* ##;s#^  ##;p') ; do
	if do_skip "$branch" "$@" ; then
		continue
	fi
	if [ -z "$(git diff "${branch}...upstream/master")" ] ; then
		continue
	fi
	do_branch "$branch" || exit $?
	echo "Done rebasing $branch"
done

if [ "$current_branch" != "$(get_current_branch)" ] ; then
	git checkout "$current_branch"
fi

# Exclude the currently checked out branch.
git branch --merged "upstream/master" | grep -v '^\* '  | xargs -r -n 1 git branch -v -d 
git branch -v
