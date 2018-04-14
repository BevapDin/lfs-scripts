#! /bin/bash

youtube-dl -f 22 "$1" -o "%(playlist_index)s - %(title)s.%(id)s.%(ext)s"
