#!/usr/bin/sudo /bin/bash
sleep 60
channels=(1 2 3 4 5 6 7 8 9 10 11 12 13 36 40 44 48 52 56 60 64 100 104 108 112 116 120 124 128 132 136 140)
mkdir $1
for i in ${channels[@]}; do
	iw wlan0 set channel ${i} HT20
	echo "channel" ${i} "set"
	outFile="$1${i}.dat"
	../netlink/log_to_file $outFile &
	./rcv_and_ack &
	python rcv_packet.py 
	sleep 5
	kill %-1
	kill $!
	outFile = "$1seq${i}.txt"
	mv seqList.txt outFile
done

