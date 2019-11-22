#!/bin/bash

dsrc_message_forward -f /etc/config/DsrcForward.conf &
sleep 1s
/usr/local/bin/IFM &
sleep 1s
