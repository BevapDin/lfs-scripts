#! /bin/bash

if amixer -c 0 get Master | grep -qE 'Playback .*\[on\]' ; then
    amixer -c 0 set Master mute
else
    amixer -c 0 set Master unmute
fi
