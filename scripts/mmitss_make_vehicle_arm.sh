#!/bin/bash
# This script finds all the Makefiles present in the rse sources, builds that 
# project for linux and copies it to the applications folder in this directory

red='\033[0;31m'

green='\033[0;32m'
nocolor='\033[0m'

################################## COMMON APPLICATIONS ################################

#######################################################################################
echo "Building Message Encoder..."
cd ./../src/common/MsgTransceiver/MsgEncoder
# Clean the folder and build for linux.
make clean &> /dev/null
make linux ARM=1 &> /dev/null

# Indicate Success/Failure of the build
if [ "$?" -eq "0" ]; then
	echo -e "${green}Successful${nocolor}"
else
	echo -e "${red}Failed${nocolor}"
fi
# Remove the .o files to keep the folders clean
rm ./*.o &> /dev/null
# Return back to original directory to go over the process again for another one
cd - &> /dev/null
#######################################################################################

#######################################################################################
echo "Building Wireless Message Decoder..."
cd ./../src/common/MsgTransceiver/MsgDecoder/WirelessMsgDecoder
# Clean the folder and build for linux.
make clean &> /dev/null
make linux ARM=1 &> /dev/null

# Indicate Success/Failure of the build
if [ "$?" -eq "0" ]; then
	echo -e "${green}Successful${nocolor}"
else
	echo -e "${red}Failed${nocolor}"
fi
# Remove the .o files to keep the folders clean
rm ./*.o &> /dev/null
# Return back to original directory to go over the process again for another one
cd - &> /dev/null
#######################################################################################

#######################################################################################
echo "Building Host BSM Decoder..."
cd ./../src/common/MsgTransceiver/MsgDecoder/HostBsmDecoder
# Clean the folder and build for linux.
make clean &> /dev/null
make linux ARM=1 &> /dev/null

# Indicate Success/Failure of the build
if [ "$?" -eq "0" ]; then
	echo -e "${green}Successful${nocolor}"
else
	echo -e "${red}Failed${nocolor}"
fi
# Remove the .o files to keep the folders clean
rm ./*.o &> /dev/null
# Return back to original directory to go over the process again for another one
cd - &> /dev/null
#######################################################################################

################################# VEHICLE APPLICATIONS ################################

#######################################################################################
echo "Building Priority Request Generator..."
cd ./../src/obu/PriorityRequestGenerator
# Clean the folder and build for linux.
make clean &> /dev/null
make linux ARM=1 &> /dev/null

# Indicate Success/Failure of the build
if [ "$?" -eq "0" ]; then
	echo -e "${green}Successful${nocolor}"
else
	echo -e "${red}Failed${nocolor}"
fi
# Remove the .o files to keep the folders clean
rm ./*.o &> /dev/null
# Return back to original directory to go over the process again for another one
cd - &> /dev/null
#######################################################################################
