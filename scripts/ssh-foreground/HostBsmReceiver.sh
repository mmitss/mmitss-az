#!/bin/bash

read -p "IP Address of target device: " targetIp
read -p "Username: " username
ssh -t -t $username@$targetIp << EOF
cd /home/mmitss/src/common/MsgTransceiver/MsgReceiver
python3 M_HostBsmReceiver.py
EOF
