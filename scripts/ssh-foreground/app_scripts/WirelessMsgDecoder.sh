#!/bin/bash

ssh -t -t $username@$targetIp << EOF
sudo su
cd /home/mmitss/src/common/MsgTransceiver/MsgDecoder/WirelessMsgDecoder
./M_WirelessMsgDecoder
EOF
