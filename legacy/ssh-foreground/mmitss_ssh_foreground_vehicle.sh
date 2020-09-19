#!/bin/bash

read -p "IP Address of RaspberryPi: " targetIp
read -p "Username on RaspberryPi: " username

export targetIp
export username

mate-terminal -e ./app_scripts/HostBsmDecoder.sh
mate-terminal -e ./app_scripts/MsgEncoder.sh
mate-terminal -e ./app_scripts/PriorityRequestGenerator.sh
mate-terminal -e ./app_scripts/WirelessMsgDecoder.sh
