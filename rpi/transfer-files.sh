#!/bin/bash

# Mac-address of smart device
MAC_ADDR="34:BB:26:A9:8C:0C"

# Calculate filename based on current date
filename=$(date +%F)

# setup connection with bluetooth device
channel=$(sdptool browse 34:BB:26:A9:8C:0C | awk 'c&&!--c;/"OBEX Object Push"/{c=4}' | grep "Channel:" | cut -d ":" -f 2)
echo "Initializing file transfer. This may take a while... "

# Plot graphs
python plottemp.py
python plot-pulse-spo2.py

# Send files
obexftp --nopath --noconn --uuid none --bluetooth $MAC_ADDR --channel $channel -p $filename"-temp.txt"
obexftp --nopath --noconn --uuid none --bluetooth $MAC_ADDR --channel $channel -p $filename"-temp.png"
obexftp --nopath --noconn --uuid none --bluetooth $MAC_ADDR --channel $channel -p $filename"-spo2.txt"
obexftp --nopath --noconn --uuid none --bluetooth $MAC_ADDR --channel $channel -p $filename"-pulse.txt"
obexftp --nopath --noconn --uuid none --bluetooth $MAC_ADDR --channel $channel -p $filename"-pulse-spo2.png"
