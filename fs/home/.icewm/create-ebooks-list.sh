find ebooks/ -type f -print | \
	sed '
		p
		s#"##g
		s#.*/##
		s#\.[^.]*$##
	' | \
	while read p && read n ; do
		echo "prog \"$n\" - mupdf \"$p\""
	done
