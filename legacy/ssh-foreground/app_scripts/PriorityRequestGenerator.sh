#!/bin/bash

ssh -t -t $username@$targetIp << EOF
sudo su
cd /home/mmitss/src/obu/PriorityRequestGenerator
./M_PriorityRequestGenerator
EOF
