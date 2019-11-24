#!/bin/bash

ssh -t -t $username@$targetIp << EOF
cd /home/mmitss/src/obu/PriorityRequestGenerator
./M_PriorityRequestGenerator
EOF
