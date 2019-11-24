#!/bin/bash

ssh -t -t $username@$targetIp << EOF
cd /home/mmitss/src/rsu/traffic-control-interface
./M_TrafficControllerInterface
EOF
