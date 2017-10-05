#!/usr/bin/sudo /bin/bash
ifconfig wlan0 down
iwconfig wlan0 mode Managed
ifconfig wlan0 up

iw dev mon0 del
