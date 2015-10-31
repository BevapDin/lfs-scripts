#! /bin/bash

find "$@" -type f -print0 | \
	xargs -0 chmod g-w,o-w,a+r
find "$@" -type d -print0 | \
	xargs -0 chmod g-w,o-w,a+x,a+r
