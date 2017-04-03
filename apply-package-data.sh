#! /bin/bash

# Best to call this script as root, as it needs to change the
# owner of the copied files to the package user.

GLOBAL_PACKAGE_USER_DIR='/usr/src'
LOCAL_PACKAGE_USER_DIR='fs/usr/src'

#VERBOSE=echo # this is verbose message on
VERBOSE=true # this is verbose messages off

apply_file() {
	local pkg="${1?Missing parameter: package name}"
	local name="${2?Missing parameter: file name}"
	local src="$LOCAL_PACKAGE_USER_DIR/$pkg/$name"
	if ! [ -e "$src" ] ; then
		return 0
	fi

	local dir="$GLOBAL_PACKAGE_USER_DIR/$pkg"
	local file="$GLOBAL_PACKAGE_USER_DIR/$pkg/$name"

	if [ -e "$file" ] ; then
		if cmp "$src" "$file" >/dev/null ; then
			$VERBOSE "Keeping $file ($src)"
			return 0
		fi
	fi
	src="$(pwd)/$src"
	$VERBOSE "Installing $src for package $pkg into $file"
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

(cd "$LOCAL_PACKAGE_USER_DIR" ; find . -mindepth 1 -maxdepth 1 -type d -printf '%f\n') | \
while read pkg ; do
	(cd "$LOCAL_PACKAGE_USER_DIR/$pkg" ; git ls-files) | \
	while read name ; do
		apply_file "$pkg" "$name" || exit $?
	done
done
