#!/bin/bash

# Define colors:
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
# Clean the folder before leaving to keep it clean for svn and/or other stuff
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
# Clean the folder before leaving to keep it clean for svn and/or other stuff
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
# Clean the folder before leaving to keep it clean for svn and/or other stuff
rm ./*.o &> /dev/null
# Return back to original directory to go over the process again for another one
cd - &> /dev/null
#######################################################################################

############################### INTERSECTION APPLICATIONS #############################

#######################################################################################
echo "Building Priority Request Server..."
cd ./../src/rsu/priority-request-server
# Clean the folder and build for linux.
make clean &> /dev/null
make linux ARM=1 &> /dev/null

# Indicate Success/Failure of the build
if [ "$?" -eq "0" ]; then
	echo -e "${green}Successful${nocolor}"
else
	echo -e "${red}Failed${nocolor}"
fi
# Clean the folder before leaving to keep it clean for svn and/or other stuff
rm ./*.o &> /dev/null
# Return back to original directory to go over the process again for another one
cd - &> /dev/null
#######################################################################################

#######################################################################################
echo "Building Priority Solver..."
cd ./../src/rsu/priority-solver
# Clean the folder and build for linux.
make clean &> /dev/null
make linux ARM=1 &> /dev/null

# Indicate Success/Failure of the build
if [ "$?" -eq "0" ]; then
	echo -e "${green}Successful${nocolor}"
else
	echo -e "${red}Failed${nocolor}"
fi
# Clean the folder before leaving to keep it clean for svn and/or other stuff
rm ./*.o &> /dev/null
# Return back to original directory to go over the process again for another one
cd - &> /dev/null
#######################################################################################

#######################################################################################
echo "Building Traffic Controller Interface..."
cd ./../src/rsu/traffic-control-interface
# Clean the folder and build for linux.
make clean &> /dev/null
make linux ARM=1 &> /dev/null

# Indicate Success/Failure of the build
if [ "$?" -eq "0" ]; then
	echo -e "${green}Successful${nocolor}"
else
	echo -e "${red}Failed${nocolor}"
fi
# Clean the folder before leaving to keep it clean for svn and/or other stuff
rm ./*.o &> /dev/null
# Return back to original directory to go over the process again for another one
cd - &> /dev/null
#######################################################################################
