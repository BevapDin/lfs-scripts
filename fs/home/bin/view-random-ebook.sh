#! /bin/bash

file="$(find $HOME/ebooks -type f -iname '*.pdf' | grep -vFi perry\ rhodan | sort -R | head -1)"
mupdf "$file"
