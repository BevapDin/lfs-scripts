#! /bin/bash

# Delete all local branches that have been merged with the upstream/master.
# upstream is usually the official repository (not your fork).
# It will ask before doing this.

# Parameters: (optional) The branch to check against.
master="${1-upstream/master}"
shift 1
if [ "$#" != 0 ] ; then
	echo "Unknown parameter: $1" 1>&2
	exit 1
fi

# Get all remote updates. One can skip this, but you'll probably want to check against the
# most recent branch information, so get it. Errors are ignored, the user might have already
# updated their repository and why fail here?
git fetch --prune upstream
git fetch --prune origin

doit() {
	local tmp="$1"
	# Exclude the currently checked out branch.
	git branch --merged "$master" | grep -v '^\* ' >"$tmp"
	if ! [ -s "$tmp" ] ; then
		echo "No branches to delete found."
		return 0
	fi
	cat "$tmp" || return $?
	while true ; do
		read -p "^^ delete those branches? [Y/N] " ans
		case "$ans" in
			Y|y|yes)
				xargs -n 1 -a "$tmp" git branch -v -d
				return $?
				;;
			N|n|no)
				return 0
				;;
		esac
	done
}

tmp="$(mktemp)"
doit "$tmp"
rm "$tmp"
# List the remaining branches that you still have to work on.
git branch
