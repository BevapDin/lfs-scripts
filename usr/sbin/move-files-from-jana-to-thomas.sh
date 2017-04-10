#! /bin/bash

doit2() {
	local src="$1"
	local dest="$2"
	if ! [ -e "$src" ] ; then
		return 0
	fi
	cd "$src" || return $?
	find . -maxdepth 1 -type f -! -iname '.*' -print0 | xargs -0 -r chown -v thomas:
	find . -maxdepth 1 -type f -! -iname '.*' -print0 | xargs -0 -r mv -n -v -t "$dest"
}

doit() {
	local user="$1"
	doit2 "/home/$user" /home/thomas
	doit2 "/var/opt/$user" /var/opt/thomas
}

doit jana
doit playing-child
