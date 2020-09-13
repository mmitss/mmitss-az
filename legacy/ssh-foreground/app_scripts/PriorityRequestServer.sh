#!/bin/bash

ssh -t -t $username@$targetIp << EOF
sudo su
cd /home/mmitss/src/rsu/priority-request-server
./M_PriorityRequestServer
EOF
