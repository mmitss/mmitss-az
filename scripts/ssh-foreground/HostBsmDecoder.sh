#!/bin/bash

read -p "IP Address of target device: " targetIp
ssh -t -t root@$targetIp << EOF
cd /home/mmitss/src/common/MsgTransceiver/MsgDecoder/HostBsmDecoder
./M_HostBsmDecoder
EOF
