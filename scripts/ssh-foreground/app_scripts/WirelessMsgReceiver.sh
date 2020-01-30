#!/bin/bash

ssh -t -t $username@$targetIp << EOF
sudo su
cd /home/mmitss/src/common/MsgTransceiver/MsgReceiver
python3 M_WirelessMsgReceiver.py
EOF
