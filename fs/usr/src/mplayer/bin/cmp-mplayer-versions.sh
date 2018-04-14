#! /bin/bash
#
# Compares two mplayer binaries based on the codes they support.

# The two mplayer binaries, first is the currently installed one (the bash sees it),
# second is the one in the current directory.
BINARY1="$(which mplayer)"
BINARY2="./mplayer"
# Command to display the comparison
VIEW_COMMAND="less -S"

usage() {
	cat <<- HERE
	Usage: $0 {<options>}
	Options:
	-1 <path> ... First binary to compare.
	-2 <path> ... Second binary to compare.
	--batch ... Only show differing lines, don't use less for display, just print the text.
HERE
}

# Only show lines that contain a difference (as reported by diff).
grep_differing() {
	grep -E '[<>|]'
}

while [ "$#" != 0 ] ; do
	opt="$1"
	shift 1
	case "$opt" in
		--batch)
			VIEW_COMMAND="grep_differing"
			;;
		-1)
			BINARY1="$1"
			shift 1
			;;
		-2)
			BINARY2="$1"
			shift 1
			;;
		--help|-h|-\?)
			usage
			exit 1
			;;
		*)
			echo "Unknown option: $opt"
			exit 1
			;;
	esac
done

# Out the codes available in binary $1, using "mplayer -$cmd help" as command line,
# cmd is the second parameter.
make_list() {
	local binary="$1"
	local cmd="$2"
	date
	echo "$binary -$cmd help"
	$binary -$cmd help 2>&1 | \
		tail -n +3 | \
		sort
}

# Create the comparison using diff, for the codes $cmd (first parameter).
do_diff() {
	local cmd="$1"
	diff -W 100 -w -y <(make_list "$BINARY1" "$cmd") <(make_list "$BINARY2" "$cmd" 2>&1)
}

# For each code:
# audio output (not actually a code), audio code, video output (not actually a code), video code
# Pipe everything to the view command.
for cmd in ao ac vo vc ; do
	do_diff "$cmd"
done | $VIEW_COMMAND

exit 0
