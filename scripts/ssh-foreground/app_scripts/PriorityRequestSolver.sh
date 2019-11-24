#!/bin/bash

ssh -t -t $username@$targetIp << EOF
cd /home/mmitss/src/rsu/priority-solver
./M_PrioritySolver
EOF
