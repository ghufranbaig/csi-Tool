#!/usr/bin/sudo /bin/bash
sleep 1
modprobe -r iwlwifi mac80211 cfg80211
modprobe iwlwifi connector_log=0x5
echo "step 1---------------"



# Setup monitor mode, loop until it works
iwconfig wlan0 mode monitor 2>/dev/null 1>/dev/null
while [ $? -ne 0 ]
do
    ifconfig wlan0 down 2>/dev/null 1>/dev/null
    iwconfig wlan0 mode monitor 2>/dev/null 1>/dev/null
done
sleep 1
echo "monitor mode successfully set"

ifconfig wlan0 up
sleep 1
echo "open wlan0"

iw dev wlan0 interface add mon0 type monitor
ifconfig mon0 up
echo "setting injection mon0"
sleep 1
echo 0x4101 | sudo tee /sys/kernel/debug/ieee80211/phy0/iwlwifi/iwldvm/debug/monitor_tx_rate

