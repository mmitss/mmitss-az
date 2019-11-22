#!/bin/bash

halt &>/dev/null

sleep 1

cp ./immediate_forward_ssm /usr/local/bin/savari/

cp ./immediate_forward /usr/local/bin/savari/

cp ./DsrcProxy_SSM.conf /etc/config/

run &>/dev/null

sleep 2

/usr/local/bin/savari/immediate_forward_ssm &

pgrep -f "immediate_forward_ssm" &>/dev/null
if [ $? -ne 0 ]; then
    echo "Failed to launch immediate_forward_ssm"
fi

killall ipv6_provider &>/dev/null
