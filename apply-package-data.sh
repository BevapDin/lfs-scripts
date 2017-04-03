#! /bin/bash

# Best to call this script as root, as it needs to change the
# owner of the copied files to the package user.

apply_file() {
	local pkg="$1"
	local name="$2"
	local src="usr/src/$pkg/$name"
	if ! [ -e "$src" ] ; then
		return 0
	fi

	local dir="/usr/src/$pkg"
	local file="/usr/src/$pkg/$name"

	if [ -e "$file" ] ; then
		if cmp "$src" "$file" >/dev/null ; then
			return 0
		fi
	fi
	src="$(pwd)/$src"
	# this may need root:
	sudo /usr/sbin/install_package "$pkg" <<EOF
	mkdir -p "${file%/*}"
	cp "$src" "$file" || exit $?
EOF
	local r="$?"
	if [ "$r" != 0 ] ; then
		return $r;
	fi
	echo "Applied '$name' file for $pkg."

	return 0
}

(cd usr/src ; find . -mindepth 1 -maxdepth 1 -type d -printf '%f\n') | \
while read pkg ; do
	(cd "usr/src/$pkg" ; git ls-files) | \
	while read name ; do
		apply_file "$pkg" "$name" || exit $?
	done	
done
