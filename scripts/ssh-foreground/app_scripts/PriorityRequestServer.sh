#!/bin/bash

ssh -t -t $username@$targetIp << EOF
cd /home/mmitss/src/rsu/priority-request-server
./M_PriorityRequestServer
EOF
