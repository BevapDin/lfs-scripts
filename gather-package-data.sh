#! /bin/bash

# This script gathers files from install users that *should*
# go into the repository.
# It collects those files, copies them into the local `usr`
# path.
# Note: the script overrides files in `usr`, make sure you
# don't have any unstaged changes.
# Usage: run after you changed something in the package user
# home folder (e.g. added a script to the personal `bin`
# folder, or changed the `.url` file).
# Afterwards look at pending changes in `usr` with git.

gather_file() {
	local pkg="$1"
	local name="$2"
	local src="/usr/src/$pkg/$name"
	if ! [ -e "$src" ] ; then
		return 0
	fi

	local dir="usr/src/$pkg"
	local file="usr/src/$pkg/$name"
	mkdir -p "$dir" || exit $?
	if [ -e "$file" ] ; then
		if cmp "$src" "$file" >/dev/null ; then
			return 0
		fi
	fi
	cp "$src" "$file" || exit $?
	echo "Added '$name' for $pkg."

	return 0
}

gather_from_dir() {
	local pkg="$1"
	local dir="$2"
	local src="/usr/src/$pkg/$dir"

	if ! [ -e "$src" ] ; then
		return 0
	fi

	find "$src" -maxdepth 1 -mindepth 1 -type f -printf '%f\n' | \
	while read file ; do
		case "$file" in
			'*~')
				continue
				;;
		esac
		gather_file "$pkg" "$dir/$file"
	done
}

find /usr/src -mindepth 1 -maxdepth 1 -type d -printf '%f\n' | \
while read pkg ; do
	gather_file "$pkg" ".url" || exit $?
done

