#! /bin/bash

if [ -e  /usr/src/ati-driver/bin/make-module.sh ] ; then
	exec sudo -u ati-driver /usr/src/ati-driver/bin/make-module.sh "$1-td"
else
	exit 0
fi

