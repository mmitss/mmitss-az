#!/bin/bash

################################## COMMON APPLICATIONS ################################

#######################################################################################
echo "Starting Message Encoder..."
cd ./../src/common/MsgTransceiver/MsgEncoder
# Clean the folder and build for linux.
./M_MsgEncoder > /dev/null 2>&1 &
# Return back to original directory to go over the process again for another one
cd - &> /dev/null

sleep 1s
#######################################################################################

#######################################################################################
echo "Starting Message Sender..."
cd ./../src/common/MsgTransceiver/MsgSender
# Clean the folder and build for linux.
python3 M_MsgSender.py > /dev/null 2>&1 &

# Return back to original directory to go over the process again for another one
cd - &> /dev/null

sleep 1s
#######################################################################################

#######################################################################################
echo "Starting Wireless Message Receiver..."
cd ./../src/common/MsgTransceiver/MsgReceiver
# Clean the folder and build for linux.
python3 M_WirelessMsgReceiver.py > /dev/null 2>&1 &

# Return back to original directory to go over the process again for another one
cd - &> /dev/null

sleep 1s
#######################################################################################

#######################################################################################
echo "Starting Wireless Message Decoder..."
cd ./../src/common/MsgTransceiver/MsgDecoder/WirelessMsgDecoder
# Clean the folder and build for linux.
./M_WirelessMsgDecoder > /dev/null 2>&1 &

# Return back to original directory to go over the process again for another one
cd - &> /dev/null

sleep 1s
#######################################################################################

############################### INTERSECTION APPLICATIONS #############################

#######################################################################################
echo "Starting Map Spat Broadcaster"
cd ./../src/mrp/map-spat-broadcaster
# Clean the folder and build for linux.
python3 M_MapSpatBroadcaster.py > /dev/null 2>&1 &

# Return back to original directory to go over the process again for another one
cd - &> /dev/null

sleep 1s
#######################################################################################

#######################################################################################
echo "Starting Priority Request Server..."
cd ./../src/rsu/priority-request-server
# Clean the folder and build for linux.
./M_PriorityRequestServer > /dev/null 2>&1 &

# Return back to original directory to go over the process again for another one
cd - &> /dev/null

sleep 1s
#######################################################################################

#######################################################################################
echo "Starting Priority Solver..."
cd ./../src/rsu/priority-solver
# Clean the folder and build for linux.
./M_PrioritySolver > /dev/null 2>&1 &

# Return back to original directory to go over the process again for another one
cd - &> /dev/null

sleep 1s
#######################################################################################

#######################################################################################
echo "Starting Traffic Controller Interface..."
cd ./../src/rsu/traffic-control-interface
# Clean the folder and build for linux.
./M_TrafficControllerInterface > /dev/null 2>&1 &

# Return back to original directory to go over the process again for another one
cd - &> /dev/null

sleep 1s
#######################################################################################
