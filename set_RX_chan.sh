#!/usr/bin/sudo /bin/bash

iw wlan0 set channel $1 HT20
echo "channel" $1 "set"

