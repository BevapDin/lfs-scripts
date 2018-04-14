#! /bin/bash

exec mplayer -playlist <(wget -O - "http://900.cloudrad.io:8138/listen.pls")
