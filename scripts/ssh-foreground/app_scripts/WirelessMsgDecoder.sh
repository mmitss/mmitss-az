#!/bin/bash

ssh -t -t $username@$targetIp << EOF
cd /home/mmitss/src/common/MsgTransceiver/MsgDecoder/WirelessMsgDecoder
./M_WirelessMsgDecoder
EOF
