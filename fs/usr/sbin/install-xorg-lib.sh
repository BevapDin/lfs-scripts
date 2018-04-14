#! /bin/bash

baseurl="http://xorg.freedesktop.org/releases/individual/lib/"
filelist="$1"

install-xorg-packages.sh --list "$filelist" "$baseurl"

