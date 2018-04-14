#! /bin/bash

for d in /usr/share/fonts/X11/* ; do
	mkfontscale "$d"
	mkfontdir "$d"
	fc-cache "$d"
done

