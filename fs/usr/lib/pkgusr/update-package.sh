#! /bin/bash

do_download=false
do_install=false

wwget() {
	local url="${1?URL is missing in wwget}"
	local file="${2?file is missing in wwget}"
	if wget --no-remove-listing -O "$file" -o /dev/null "$url" ; then
		return 0
	fi
	ret="$?"
	echo "WGet failed for URL $url" 1>&2
	exit $ret
}

usage () {
	echo "$0 <options>*"
	echo "Options:"
	echo "-d ... download most recent version"
	echo "-i ... install most recent version (implies -d)"
	exit 1
}

while [ $# != 0 ] ; do
	opt="$1"
	shift 1
	case "$opt" in
		-\?|--help|-h)
			usage
			;;
		-d)
			do_download=true
			;;
		-i)
			do_install=true
			do_download=true
			;;
		*)
			echo "Unknown option: $opt" 1>&2
			usage
			exit 1
			;;
	esac
done

current_version="$(version)"
if [ $? != 0 -o -z "$current_version" ] ; then
	echo "Could not determine the current version" 1>&2
	exit 1
fi

getver() {
	local l=".listing"
	local url="${1?missing url in getver}"
	local search="${2?missing search in getver}"
	local replace="${3?missing replace in getver}"
	if [ -e "$l" ] ; then
		rm "$l" || return $?
	fi
	wwget "$url" /dev/null || return $?
	if ! [ -e "$l" ] ; then
		echo "Missing $l after wget" 1>&2
		return 1
	fi

	exver "$search" "$replace" < "$l"
	return $?
}

exver() {
	local search="${1?missing search in exver}"
	local replace="${2?missing replace in exver}"
	grep -Ee "$search" | \
		sed "$replace" | \
		sort -V | \
		tail -1
}

xexver() {
	exver '[0-9a-z]\.[0-9a-z][0-9a-z]*\.[0-9-az]' \
		's#^.*\([0-9a-z][0-9a-z]*\.[0-9a-z][0-9a-z]*(\.[0-9-az][0-9a-z]*)*\).*#\1#'
}

std_untar() {
	local file="${1?missing file in std_untar}"
	tar -xf "$file" >/dev/null || return $?
	dir="${file%.tar*}"
	if ! [ -e "$dir" ] ; then
		echo "Tar did not create the expected folder: $dir from $file" 1>&2
		return 1
	fi
	echo "$dir"
}

cfgc_update() {
	local defconf="${1?Missing defconf in cfgc_update}"
	shift 1
	local cfgcconf="$PWD/myconfig"
	if ! [ -e "$defconf" ] ; then
		echo "defconf file does not exist: $defconf" 1>&2
		return 1
	fi
	cfgc --quiet --no-configure "$@" || return $?
	sed -i '/^[^#]/ { s#^#\# #; } ' "$cfgcconf" || return $?
	echo >> "$cfgcconf" || return $?
	grep -E '^[^#]' < "$defconf" >> "$cfgcconf" || return $?
	cfgc --quiet --no-install || return $?
	tpkgs --uninstall || return $?
	if make install ; then
		mv-pkg-version-dir.sh "$v2" || return $?
		return 0
	fi
	ret="$?"
	tpkgs --install
	return $ret
}

std_download_install() {
	local url="${1?missing url in std_download_install}"
	local file="${2?missing file in std_download_install}"
	local defconf="${3?missing defonf in std_download_install}"

	if [ -z "$file" ] ; then
		file="${url/*\/}"
	fi
	if [ -z "$defconf" ] ; then
		defconf="$(ls -1 $HOME/.*.config | sort -V | tail -1)"
		local ret="$?"
		if [ $ret != 0 -o -z "$defconf" ] ; then
			echo "No defualt config file found" 1>&2
			return $ret
		fi
	fi

    wget -o /dev/null -O "$file" "$url" || return $?
    local dir="$(std_untar "$file")"
    if [ $? != 0 -o -z "$dir" ] ; then
        return $?
    fi
    cd "$dir" || return $?
    cfgc_update "$defconf" || return "$?"
    cd ..
    mk-ok-tar "$dir"
}

update_config_file="$HOME/.update.config.sh"
if ! [ -e "$update_config_file" -a -x "$update_config_file" ] ; then
	echo "Missing config file: $update_config_file" 1>&2
	exit 1
fi

. "$update_config_file"

if [ -z "$get_new_version_function" ] ; then
	echo "Missing config entry get_new_version_function" 1>&2
	exit 1
fi
if [ -z "$download_function" ] ; then
	echo "Missing config entry download_function" 1>&2
	exit 1
fi
if [ -z "$download_and_install_function" ] ; then
	echo "Missing config entry download_and_install_function" 1>&2
	exit 1
fi

new_version="$($get_new_version_function)"
if [ $? != 0 -o -z "$new_version" ] ; then
	echo "Could not get new version" 1>&2
	exit 1
fi

if [ "$current_version" == "$new_version" ] ; then
	echo "Package is up to date (version $current_version)"
	exit 0
fi

echo "Newer version available: $current_version ==> $new_version!"

if ! $do_download ; then
	exit 0
fi

if $do_install ; then
	$download_and_install_function
else
	$download_function
fi

