#!/usr/bin/sudo /bin/bash
echo "start configure---------------------"
modprobe -r iwlwifi mac80211 cfg80211
modprobe iwlwifi debug=0x40000
sleep 1

ifconfig wlan0 2>/dev/null 1>/dev/null
while [ $? -ne 0 ]
do 
            ifconfig wlan0 2>/dev/null 1>/dev/null
done
echo "setting injection mode ok"
sleep 1
iw dev mon0 del 2>/dev/null 1>/dev/null
iw dev wlan0 interface add mon0 type monitor
ifconfig mon0 up
echo "setting injection mon0"
sleep 1
echo 0x4101 | sudo tee /sys/kernel/debug/ieee80211/phy0/iwlwifi/iwldvm/debug/monitor_tx_rate

