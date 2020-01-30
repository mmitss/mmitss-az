#!/bin/bash

ssh -t -t $username@$targetIp << EOF
sudo su
cd /home/mmitss/src/rsu/priority-solver
./M_PrioritySolver
EOF
