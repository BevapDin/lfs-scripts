#! /bin/bash

signal="$1"

ps -A --format 'pid %cpu %mem euser:33 command' k -%cpu -w -w | \
while read pid cpu mem user com ; do
	if [ "$cpu" = '0.0' ] ; then
		continue
	fi
	name="${com%% *}"
	name="${name##*/}"
	case "$user" in
		playling-child|jana|thomas)
			echo "prog '$name ($com, $cpu, $mem)' - sudo -u $user /bin/bash kill -$signal $pid"
			;;
	esac
done
