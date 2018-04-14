#! /bin/bash

doit() {
	local file="/var/opt/thomas/$1"
	local url="$2"
	if [ -e "$file.tmp" ] ; then
		mv --backup=numbered "$file.tmp" "$file"
	fi
	while true ; do
		mplayer -dumpstream -dumpfile "$file.tmp" "$url"
		mv --backup=numbered "$file.tmp" "$file"
	done
}

doit dradio-wissen 'http://dradio_mp3_dwissen_m.akacast.akamaistream.net/7/728/142684/v1/gnl.akacast.akamaistream.net/dradio_mp3_dwissen_m' &
doit dradio-kultur 'http://dradio-ogg-dkultur-l.akacast.akamaistream.net/7/978/135496/v1/gnl.akacast.akamaistream.net/dradio_ogg_dkultur_l' &
doit dradio 'http://dradio-ogg-dlf-l.akacast.akamaistream.net/7/629/135496/v1/gnl.akacast.akamaistream.net/dradio_ogg_dlf_l' &
doit dylan 'http://900.cloudrad.io:8138/' &
