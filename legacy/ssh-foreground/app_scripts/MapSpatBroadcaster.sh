#!/bin/bash

ssh -t -t $username@$targetIp << EOF
sudo su
cd /home/mmitss/src/mrp/map-spat-broadcaster/
python3 M_MapSpatBroadcaster.py
EOF
