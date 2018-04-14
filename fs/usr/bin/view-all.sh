#! /bin/bash

handled="txt|nfo"

ff() {
	local p="$1"
	local m="$2"
	shift 2
	if [ "$1" = '--size' ] ; then
		shift 1
		xargs -a <(find "$@" -type f -! -empty -printf '%s %p\n' | grep -iE "\\.($p)\$" | sort -n | sed 's#^[0-9]* ##') -d '\n' -r $m
	elif [ "$1" = '--rsize' ] ; then
		shift 1
		xargs -a <(find "$@" -type f -! -empty -printf '%s %p\n' | grep -iE "\\.($p)\$" | sort -nr | sed 's#^[0-9]* ##') -d '\n' -r $m
	elif [ "$1" = '--shuffle' ] ; then
		shift 1
		xargs -a <(find "$@" -type f -! -empty -print | grep -iE "\\.($p)\$" | sort -R) -d '\n' -r $m
	else
		xargs -a <(find "$@" -type f -! -empty -print | grep -iE "\\.($p)\$" | sort) -d '\n' -r $m
	fi
	handled="$handled|$p"
}

ff 'gif|jpg|jpeg|png' gpicview "$@"
ff 'mpeg|avi|mpg|wmv|mp4|asf|divx|mov|flv' 'mplayer -fixed-vo -fs -osdlevel 1' "$@"

if [ "$1" = '--size' ] ; then
	shift 1
elif [ "$1" = '--rsize' ] ; then
	shift 1
elif [ "$1" = '--shuffle' ] ; then
	shift 1
fi
find "$@" -type f -print | grep -viE "\\.($handled)\$"
