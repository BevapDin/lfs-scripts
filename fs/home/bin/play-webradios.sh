#! /bin/bash

playlists() {
	# DRadio Kultur
	wget -O - 'http://www.deutschlandradio.de/streaming/dkultur_hq_ogg.m3u'
	# DRadio
	wget -O - 'http://www.deutschlandradio.de/streaming/dlf_hq_ogg.m3u'
	# DRadio Wissen
	echo 'http://dradio_mp3_dwissen_m.akacast.akamaistream.net/7/728/142684/v1/gnl.akacast.akamaistream.net/dradio_mp3_dwissen_m'
	# Bob Dylan
	# wget -O - 'http://900.cloudrad.io:8138/listen.pls'
	wget -O - 'http://www.dylanradio.com/listen.m3u'
}

exec gmplayer -playlist <(playlists)
