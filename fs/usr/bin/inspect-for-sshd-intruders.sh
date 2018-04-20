#! /bin/bash

tty

sed -r '/sshd/ !d; s,^.*([0-9]+\.[0-9]+\.[0-9]+\.[0-9]+).*,\1,;t;d' /var/log/all.log | \
	uniq | \
	sort | \
	grep -v '141.30.4.114' | \
	grep -v '0.0.0.0' | \
	uniq |
	while read ip ; do
		echo "IP: $ip"
		nmap -A "$ip"
		echo -e "\n\n------------\n"
	done

