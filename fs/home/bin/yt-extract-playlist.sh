#! /bin/bash

file="$1"
grep -oE '"[^"]*index=[^"]*list=[^"]*"' "$file" | tr -d '"' | \
    sed '
        s#&amp;#\&#g;
        s#?\(.*\)&\(index=[0-9]*\)#?\2\&\1#
    ' | sort -t = -k 2 -n | uniq
