#! /bin/bash

inp="/caesar/home/thomas/playlist.m3u"
out="/mnt/usb/mp3"

while true ; do
	file="$(sort -R <"$inp"|head -1)"
	echo "$file?"
	if read a && [ a == 'y' ] ; then
		cp "$file" "$out" || break
	fi
done
