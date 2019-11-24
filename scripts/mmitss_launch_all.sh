#!/bin/bash

################################## COMMON APPLICATIONS ################################

#######################################################################################
echo "Starting Message Encoder..."
cd ./../src/common/MsgTransceiver/MsgEncoder
./M_MsgEncoder > /dev/null 2>&1 &
# Return back to original directory to go over the process again for another one
cd - &> /dev/null
sleep 1s
#######################################################################################

#######################################################################################
echo "Starting Message Sender..."
cd ./../src/common/MsgTransceiver/MsgSender
python3 M_MsgSender.py > /dev/null 2>&1 &

# Return back to original directory to go over the process again for another one
cd - &> /dev/null
sleep 1s
#######################################################################################

#######################################################################################
echo "Starting Wireless Message Receiver..."
cd ./../src/common/MsgTransceiver/MsgReceiver
python3 M_WirelessMsgReceiver.py > /dev/null 2>&1 &

# Return back to original directory to go over the process again for another one
cd - &> /dev/null
sleep 1s
#######################################################################################

#######################################################################################
echo "Starting Wireless Message Decoder..."
cd ./../src/common/MsgTransceiver/MsgDecoder/WirelessMsgDecoder
./M_WirelessMsgDecoder > /dev/null 2>&1 &

# Return back to original directory to go over the process again for another one
cd - &> /dev/null
sleep 1s
#######################################################################################

#######################################################################################
echo "Starting Host BSM Receiver..."
cd ./../src/common/MsgTransceiver/MsgReceiver
python3 M_HostBsmReceiver.py > /dev/null 2>&1 &

# Return back to original directory to go over the process again for another one
cd - &> /dev/null
sleep 1s
#######################################################################################

#######################################################################################
echo "Starting Host BSM Decoder..."
cd ./../src/common/MsgTransceiver/MsgDecoder/HostBsmDecoder
./M_HostBsmDecoder > /dev/null 2>&1 &

# Return back to original directory to go over the process again for another one
cd - &> /dev/null
sleep 1s
#######################################################################################

################################# VEHICLE APPLICATIONS ################################

#######################################################################################
echo "Starting Priority Request Generator..."
cd ./../src/obu/PriorityRequestGenerator
./M_PriorityRequestGenerator > /dev/null 2>&1 &

# Return back to original directory to go over the process again for another one
cd - &> /dev/null
sleep 1s
#######################################################################################

############################### INTERSECTION APPLICATIONS #############################

#######################################################################################
echo "Starting Map Spat Broadcaster"
cd ./../src/mrp/map-spat-broadcaster
python3 M_MapSpatBroadcaster.py > /dev/null 2>&1 &

# Return back to original directory to go over the process again for another one
cd - &> /dev/null
sleep 1s
#######################################################################################

#######################################################################################
echo "Starting Priority Request Server..."
cd ./../src/rsu/priority-request-server
./M_PriorityRequestServer > /dev/null 2>&1 &

# Return back to original directory to go over the process again for another one
cd - &> /dev/null
sleep 1s
#######################################################################################

#######################################################################################
echo "Starting Priority Solver..."
cd ./../src/rsu/priority-solver
./M_PrioritySolver > /dev/null 2>&1 &

# Return back to original directory to go over the process again for another one
cd - &> /dev/null
sleep 1s
#######################################################################################

#######################################################################################
echo "Starting Traffic Controller Interface..."
cd ./../src/rsu/traffic-control-interface
./M_TrafficControllerInterface > /dev/null 2>&1 &

# Return back to original directory to go over the process again for another one
cd - &> /dev/null
sleep 1s
#######################################################################################