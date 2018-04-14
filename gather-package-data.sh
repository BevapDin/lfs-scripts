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

GLOBAL_PACKAGE_USER_DIR='/usr/src'
LOCAL_PACKAGE_USER_DIR='fs/usr/src'

gather_file() {
	local pkg="${1?Missing parameter: package name}"
	local name="${2?Missing parameter: file name}"
	local src="$GLOBAL_PACKAGE_USER_DIR/$pkg/$name"
	if ! [ -e "$src" ] ; then
		return 0
	fi

	local dir="$LOCAL_PACKAGE_USER_DIR/$pkg"
	local file="$LOCAL_PACKAGE_USER_DIR/$pkg/$name"
	mkdir -p "$dir" || exit $?
	if [ -e "$file" ] ; then
		if cmp "$src" "$file" >/dev/null ; then
			return 0
		fi
	fi
	mkdir -p "${file%/*}"
	cp "$src" "$file" || exit $?
	echo "Added '$name' for $pkg."

	return 0
}

gather_from_dir() {
	local pkg="${1?Missing parameter: package name}"
	local dir="${2?Missing parameter: directory name}"
	local src="$GLOBAL_PACKAGE_USER_DIR/$pkg/$dir"

	if ! [ -e "$src" ] ; then
		return 0
	fi

	find "$src" -maxdepth 1 -mindepth 1 -type f -printf '%f\n' | \
	while read file ; do
		# List files that do not belong into the repository here:
		case "$file" in
			'*~')
				continue
				;;
		esac
		# and files that belong into it here:
		gather_file "$pkg" "$dir/$file"
	done
}

find "$GLOBAL_PACKAGE_USER_DIR" -mindepth 1 -maxdepth 1 -type d -printf '%f\n' | \
while read pkg ; do
	# List all the files you want to gather here:
	# File with URL used by the `new` utility.
	gather_file "$pkg" ".url" || exit $?
	# File with package specific notes.
	gather_file "$pkg" "notes" || exit $?
done
