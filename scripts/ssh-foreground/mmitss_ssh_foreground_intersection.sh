#!/bin/bash

read -p "IP Address of CVCP: " targetIp
read -p "Username on CVCP: " username

export targetIp
export username

mate-terminal -e ./app_scripts/MapSpatBroadcaster.sh
mate-terminal -e ./app_scripts/MsgEncoder.sh
mate-terminal -e ./app_scripts/MsgSender.sh
mate-terminal -e ./app_scripts/PriorityRequestServer.sh
mate-terminal -e ./app_scripts/PriorityRequestSolver.sh
mate-terminal -e ./app_scripts/TrafficControllerInterface.sh
mate-terminal -e ./app_scripts/WirelessMsgDecoder.sh
mate-terminal -e ./app_scripts/WirelessMsgReceiver.sh

