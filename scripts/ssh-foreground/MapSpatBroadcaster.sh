#!/bin/bash

read -p "IP Address of target device: " targetIp
read -p "Username: " username
ssh -t -t $username@$targetIp << EOF
cd /home/mmitss/src/mrp/map-spat-broadcaster/
python3 M_MapSpatBroadcaster.py
EOF
