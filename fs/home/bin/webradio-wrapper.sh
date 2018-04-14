#! /bin/bash

url="invalid"
file="$1"

case "$2" in
	dradio-wissen)
		url='http://dradio_mp3_dwissen_m.akacast.akamaistream.net/7/728/142684/v1/gnl.akacast.akamaistream.net/dradio_mp3_dwissen_m'
		;;
	dradio-kultur)
		url='http://dradio-ogg-dkultur-l.akacast.akamaistream.net/7/978/135496/v1/gnl.akacast.akamaistream.net/dradio_ogg_dkultur_l'
		;;
	dradio)
		url='http://dradio-ogg-dlf-l.akacast.akamaistream.net/7/629/135496/v1/gnl.akacast.akamaistream.net/dradio_ogg_dlf_l'
		;;
	dylan)
		url='http://900.cloudrad.io:8138/'
		;;
	*)
		echo "Unknown channel" 1>&2
		sleep 1
		exit 1
esac

exec mplayer -really-quiet -quiet -dumpstream -dumpfile "$file" "$url"
