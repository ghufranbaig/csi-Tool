#!/usr/bin/sudo /bin/bash

iw mon0 set channel $1 HT20
echo "channel" $1 "set"
