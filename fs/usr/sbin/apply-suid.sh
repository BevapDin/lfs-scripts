#! /bin/bash

INPUT_LIST=/etc/suid-programs

echo "Making files in $INPUT_LIST suid root."

grep -v '^#' "$INPUT_LIST" | \
while read path ; do
	if [ ! -f "$path" ] ; then
		echo "Error: $path does not exist." 1>&2
		continue
	fi
	ls -l "$path"
	chown root "$path" || exit $?
	chmod u+s "$path" || exit $?
done

exit 0
