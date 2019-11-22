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

#######################################################################################
echo "Starting Host BSM Receiver..."
cd ./../src/common/MsgTransceiver/MsgReceiver
# Clean the folder and build for linux.
python3 M_HostBsmReceiver.py > /dev/null 2>&1 &

# Return back to original directory to go over the process again for another one
cd - &> /dev/null

sleep 1s
#######################################################################################

#######################################################################################
echo "Starting Host BSM Decoder..."
cd ./../src/common/MsgTransceiver/MsgDecoder/HostBsmDecoder
# Clean the folder and build for linux.
./M_HostBsmDecoder > /dev/null 2>&1 &

# Return back to original directory to go over the process again for another one
cd - &> /dev/null

sleep 1s
#######################################################################################

################################# VEHICLE APPLICATIONS ################################

#######################################################################################
echo "Starting Priority Request Generator..."
cd ./../src/obu/PriorityRequestGenerator
# Clean the folder and build for linux.
./M_PriorityRequestGenerator > /dev/null 2>&1 &

# Return back to original directory to go over the process again for another one
cd - &> /dev/null

sleep 1s
#######################################################################################
