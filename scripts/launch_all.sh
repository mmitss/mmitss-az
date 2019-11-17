#!/bin/bash
# This script finds all the Makefiles present in the rse sources, builds that 
# project for linux and copies it to the applications folder in this directory

red='\033[0;31m'

green='\033[0;32m'
nocolor='\033[0m'

################################## COMMON APPLICATIONS ################################

#######################################################################################
echo "Message Encoder..."
cd ./../src/common/MsgTransceiver/MsgEncoder
# Clean the folder and build for linux.
./TransceiverEncoder > /dev/null 2>&1 &

# Indicate Success/Failure of the build
if [ "$?" -eq "0" ]; then
	echo -e "${green}Started successfully${nocolor}"
else
	echo -e "${red}Failed to start${nocolor}"
fi
# Clean the folder before leaving to keep it clean for svn and/or other stuff
rm ./*.o > /dev/null 2>&1 &
# Return back to original directory to go over the process again for another one
cd - &> /dev/null

sleep 1s
#######################################################################################

#######################################################################################
echo "Message Sender..."
cd ./../src/common/MsgTransceiver/MsgSender
# Clean the folder and build for linux.
python3 MsgSender.py > /dev/null 2>&1 &

# Indicate Success/Failure of the build
if [ "$?" -eq "0" ]; then
	echo -e "${green}Started successfully${nocolor}"
else
	echo -e "${red}Failed to start${nocolor}"
fi
# Clean the folder before leaving to keep it clean for svn and/or other stuff
rm ./*.o > /dev/null 2>&1 &
# Return back to original directory to go over the process again for another one
cd - &> /dev/null

sleep 1s
#######################################################################################

#######################################################################################
echo "Wireless Message Receiver..."
cd ./../src/common/MsgTransceiver/MsgReceiver
# Clean the folder and build for linux.
python3 wireless-msg-receiver.py > /dev/null 2>&1 &

# Indicate Success/Failure of the build
if [ "$?" -eq "0" ]; then
	echo -e "${green}Started successfully${nocolor}"
else
	echo -e "${red}Failed to start${nocolor}"
fi
# Clean the folder before leaving to keep it clean for svn and/or other stuff
rm ./*.o > /dev/null 2>&1 &
# Return back to original directory to go over the process again for another one
cd - &> /dev/null

sleep 1s
#######################################################################################

#######################################################################################
echo "Wireless Message Decoder..."
cd ./../src/common/MsgTransceiver/MsgDecoder/WirelessMsgDecoder
# Clean the folder and build for linux.
./WirelessMsgDecoder > /dev/null 2>&1 &

# Indicate Success/Failure of the build
if [ "$?" -eq "0" ]; then
	echo -e "${green}Started successfully${nocolor}"
else
	echo -e "${red}Failed to start${nocolor}"
fi
# Clean the folder before leaving to keep it clean for svn and/or other stuff
rm ./*.o > /dev/null 2>&1 &
# Return back to original directory to go over the process again for another one
cd - &> /dev/null

sleep 1s
#######################################################################################

#######################################################################################
echo "Host BSM Receiver..."
cd ./../src/common/MsgTransceiver/MsgReceiver
# Clean the folder and build for linux.
python3 host-bsm-receiver.py > /dev/null 2>&1 &

# Indicate Success/Failure of the build
if [ "$?" -eq "0" ]; then
	echo -e "${green}Started successfully${nocolor}"
else
	echo -e "${red}Failed to start${nocolor}"
fi
# Clean the folder before leaving to keep it clean for svn and/or other stuff
rm ./*.o > /dev/null 2>&1 &
# Return back to original directory to go over the process again for another one
cd - &> /dev/null

sleep 1s
#######################################################################################

#######################################################################################
echo "Host BSM Decoder..."
cd ./../src/common/MsgTransceiver/MsgDecoder/HostBsmDecoder
# Clean the folder and build for linux.
./HostBsmDecoder > /dev/null 2>&1 &

# Indicate Success/Failure of the build
if [ "$?" -eq "0" ]; then
	echo -e "${green}Started successfully${nocolor}"
else
	echo -e "${red}Failed to start${nocolor}"
fi
# Clean the folder before leaving to keep it clean for svn and/or other stuff
rm ./*.o > /dev/null 2>&1 &
# Return back to original directory to go over the process again for another one
cd - &> /dev/null

sleep 1s
#######################################################################################

################################# VEHICLE APPLICATIONS ################################

#######################################################################################
echo "Priority Request Generator..."
cd ./../src/obu/PriorityRequestGenerator
# Clean the folder and build for linux.
./PRG > /dev/null 2>&1 &

# Indicate Success/Failure of the build
if [ "$?" -eq "0" ]; then
	echo -e "${green}Started successfully${nocolor}"
else
	echo -e "${red}Failed to start${nocolor}"
fi
# Clean the folder before leaving to keep it clean for svn and/or other stuff
rm ./*.o > /dev/null 2>&1 &
# Return back to original directory to go over the process again for another one
cd - &> /dev/null

sleep 1s
#######################################################################################

############################### INTERSECTION APPLICATIONS #############################

#######################################################################################
echo "Map Spat Broadcaster"
cd ./../src/mrp/map-spat-broadcaster
# Clean the folder and build for linux.
python3 MapSpatBroadcaster.py > /dev/null 2>&1 &

# Indicate Success/Failure of the build
if [ "$?" -eq "0" ]; then
	echo -e "${green}Started successfully${nocolor}"
else
	echo -e "${red}Failed to start${nocolor}"
fi
# Clean the folder before leaving to keep it clean for svn and/or other stuff
rm ./*.o > /dev/null 2>&1 &
# Return back to original directory to go over the process again for another one
cd - &> /dev/null

sleep 1s
#######################################################################################

#######################################################################################
echo "Priority Request Server..."
cd ./../src/rsu/priority-request-server
# Clean the folder and build for linux.
./M_PriorityRequestServer > /dev/null 2>&1 &

# Indicate Success/Failure of the build
if [ "$?" -eq "0" ]; then
	echo -e "${green}Started successfully${nocolor}"
else
	echo -e "${red}Failed to start${nocolor}"
fi
# Clean the folder before leaving to keep it clean for svn and/or other stuff
rm ./*.o > /dev/null 2>&1 &
# Return back to original directory to go over the process again for another one
cd - &> /dev/null

sleep 1s
#######################################################################################

#######################################################################################
echo "Priority Solver..."
cd ./../src/rsu/priority-solver
# Clean the folder and build for linux.
./M_PrioritySolver > /dev/null 2>&1 &

# Indicate Success/Failure of the build
if [ "$?" -eq "0" ]; then
	echo -e "${green}Started successfully${nocolor}"
else
	echo -e "${red}Failed to start${nocolor}"
fi
# Clean the folder before leaving to keep it clean for svn and/or other stuff
rm ./*.o > /dev/null 2>&1 &
# Return back to original directory to go over the process again for another one
cd - &> /dev/null

sleep 1s
#######################################################################################

#######################################################################################
echo "Traffic Controller Interface..."
cd ./../src/rsu/traffic-control-interface
# Clean the folder and build for linux.
./M_TrafficControllerInterface > /dev/null 2>&1 &

# Indicate Success/Failure of the build
if [ "$?" -eq "0" ]; then
	echo -e "${green}Started successfully${nocolor}"
else
	echo -e "${red}Failed to start${nocolor}"
fi
# Clean the folder before leaving to keep it clean for svn and/or other stuff
rm ./*.o > /dev/null 2>&1 &
# Return back to original directory to go over the process again for another one
cd - &> /dev/null

sleep 1s
#######################################################################################
