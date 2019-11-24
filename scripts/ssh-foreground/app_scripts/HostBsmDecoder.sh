#!/bin/bash

ssh -t -t $username@$targetIp << EOF
cd /home/mmitss/src/common/MsgTransceiver/MsgDecoder/HostBsmDecoder
./M_HostBsmDecoder
EOF
