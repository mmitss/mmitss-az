#!/bin/bash

mate-terminal -e ./HostBsmDecoder.sh
mate-terminal -e ./HostBsmReceiver.sh
mate-terminal -e ./MsgEncoder.sh
mate-terminal -e ./MsgSender.sh
mate-terminal -e ./PriorityRequestGenerator.sh
mate-terminal -e ./WirelessMsgDecoder.sh
mate-terminal -e ./WirelessMsgReceiver.sh

