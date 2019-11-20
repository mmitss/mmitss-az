#!/bin/bash

pkill -9 -f M_TrafficControllerInterface 
pkill -9 -f PRG 
pkill -9 -f TransceiverEncoder 
pkill -9 -f MapSpatBroadcaster 
pkill -9 -f HostBsmDecoder 
pkill -9 -f WirelessMsgDecoder 
pkill -9 -f host-bsm-receiver 
pkill -9 -f wireless-msg-receiver 
pkill -9 -f MsgSender 
pkill -9 -f M_PrioritySolver 
pkill -9 -f M_PriorityRequestServer

