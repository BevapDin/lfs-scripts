#!/bin/bash

if ! [ -z "$1" ] ; then
	exec git stash show --patch "$1"
fi

for y in `git stash list | cut -d":" -f1` ; do
	echo $'\e[1;33m'$y$'\e[0m'
	git stash show $y "$@"
done

