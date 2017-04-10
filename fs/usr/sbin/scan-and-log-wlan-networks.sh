#! /bin/bash

log=/root/wlan-networks

echo -ne 'scan_results\nq\n' | wpa_cli | grep -vE '^>' | \
grep -E '^(bssid|..:..:..:..)' >/tmp/$$.1

cat "$log" /tmp/$$.1 | sort | uniq -w 17 >/tmp/$$.2 && mv /tmp/$$.2 "$log"

rm /tmp/$$.1
