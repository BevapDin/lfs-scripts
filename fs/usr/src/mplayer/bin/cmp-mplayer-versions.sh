#! /bin/bash

BIN1="$(which mplayer)"
BIN2="./mplayer"
LOG="/tmp/mplayer-cmp-$$-"
out_cmd="less -S"
store_results=""

resumefnc() {
	grep -E '[<>|]'
}

while [ "$#" != 0 ] ; do
	opt="$1"
	shift 1
	case "$opt" in
		--store)
			store_results="$1"
			shift 1
			;;
		--batch)
			out_cmd="resumefnc"
			;;
		*)
			echo "Unknown option: $opt"
			exit 1
			;;
	esac
done

makelist() {
	local binary="$1"
	local cmd="$2"
	date
	echo "$binary $cmd"
	$binary -$cmd help 2>&1 | \
		tail -n +3 | \
		sort
}

for d in ao ac vo vc ; do
	l1="$LOG-$d.1"
	l2="$LOG-$d.2"
	makelist "$BIN1" "$d" > "$l1"
	makelist "$BIN2" "$d" > "$l2"
	diff -W 100 -w -y "$l1" "$l2" 2>&1 | $out_cmd
	if [ -n "$store_results" ] ; then
		cp "$l2" "$store_results"
	fi
	rm "$l1" "$l2"
done

exit 0
