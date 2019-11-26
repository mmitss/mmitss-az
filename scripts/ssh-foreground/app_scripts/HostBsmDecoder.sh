#!/bin/bash

ssh -t -t $username@$targetIp << EOF
sudo su
cd /home/mmitss/src/common/MsgTransceiver/MsgDecoder/HostBsmDecoder
./M_HostBsmDecoder
EOF
