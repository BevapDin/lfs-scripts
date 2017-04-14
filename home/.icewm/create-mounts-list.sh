sed '
	/^ *#/d;           # comments
	/^$/d;             # empty lines
 	/^[a-z]* /d;       # pseudo file systems
	/ swap /d;
	/ \/ /d;
	' /etc/fstab | \
	while read dev mp blob ; do
		if ! [ -e "$dev" ] ; then
			continue
		fi
		if ! [ -e "$mp" ] ; then
			continue
		fi
		if mount | grep -qF "$mp" ; then
			echo "prog \"Unmount $mp\" - umount \"$dev\""
		else
			echo "prog \"Mount $mp\" - mount \"$dev\""
		fi
	done | \
	sort
