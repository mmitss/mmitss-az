#!/bin/bash

ssh -t -t $username@$targetIp << EOF
sudo su
cd /home/mmitss/src/rsu/traffic-control-interface
./M_TrafficControllerInterface
EOF
