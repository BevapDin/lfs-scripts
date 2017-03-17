#! /bin/bash

# This script is intended to be run by `cron` every few days or so.
# It scans the log file for entries from `sshd`, extracts the IP and
# attempts to scan those IPs via `nmap`.
# This assumes the IP are attackers that try to "hack" into the local
# system.

LOG=/var/log/all.log
OUTPUT_DIR="$HOME/namp-results"

if ! which nmap 2>/dev/null 1>/dev/null ; then
	echo 'This script needs the `nmap` program, please install it or adjust $PATH.' 1>&2
	exit 1
fi

if ! [ -e "$LOG" ] ; then
	echo "The log file $LOG does no exist." 1>&2
	exit 1
fi

sed -r '/sshd/ !d; s,^.*[^0-9]([0-9]+\.[0-9]+\.[0-9]+\.[0-9]+)[^0-9].*,\1,;t;d' "$LOG" | \
	uniq | \
	sort | \
	grep -v '0.0.0.0' | \
	uniq |
	while read ip ; do
		out="$OUTPUT_DIR/$ip/nmap"
		if [ -e "$out" ] ; then
			echo "IP $ip is already scaned, see $out"
		else
			echo "Scanning IP $ip"
			mkdir -p "${out%/*}" || exit $?
			nmap -A "$ip" | tee "$out"
			echo -e "------------\n\n"
		fi
	done

