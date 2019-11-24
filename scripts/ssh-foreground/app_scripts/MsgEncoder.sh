#!/bin/bash

ssh -t -t $username@$targetIp << EOF
cd /home/mmitss/src/common/MsgTransceiver/MsgEncoder
./M_MsgEncoder
EOF
