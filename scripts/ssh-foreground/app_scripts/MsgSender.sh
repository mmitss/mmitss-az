#!/bin/bash

ssh -t -t $username@$targetIp << EOF
sudo su
cd /home/mmitss/src/common/MsgTransceiver/MsgSender
python3 M_MsgSender.py
EOF
