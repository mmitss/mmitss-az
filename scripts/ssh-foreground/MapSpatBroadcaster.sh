#!/bin/bash

read -p "IP Address of target device: " targetIp
ssh -t -t root@$targetIp << EOF
cd /home/mmitss/src/mrp/map-spat-broadcaster/
python3 M_MapSpatBroadcaster.py
EOF
