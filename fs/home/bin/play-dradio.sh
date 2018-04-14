#! /bin/bash

exec mplayer -playlist <(wget -O - http://www.deutschlandradio.de/streaming/dlf_hq_ogg.m3u)
