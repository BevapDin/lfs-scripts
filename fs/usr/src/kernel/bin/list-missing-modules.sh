#! /bin/bash

# List the modules of the kernel of the given version (1. param is version)
listmodules() {
	local kvers="$1"
	local path="/lib/modules/$kvers/kernel"
	if ! [ -d "$path" ] ; then
		echo "Missing kernel module dir for $kvers: $path" 1>&2
		return 1
	fi
	echo "$kvers:"
	cd "/lib/modules/$kvers/kernel" || return 1
	find . -type f
}

# Compare the list of kernel modules of the currently active kernel and the new
# kernel (the version of the new kernel is given as 1. param).
list_missing_modules() {
	local v1="$1"
	local v2="$2"
	local tmpfile="$(mktemp)"
	diff -y <(listmodules "$v1") <(listmodules "$v2") | \

	grep -E '[<>|]' > "$tmpfile"
	if [ "$(wc -l "$tmpfile" | cut -d ' ' -f 1)" -gt 1 ] ; then
		echo "Don't forget additional modules:"
		cat "$tmpfile"
	else
		echo "No additional modules!"
	fi
	rm "$tmpfile"
}

list_missing_modules "${1?missing version 1}" "${2?missing version 2}"

