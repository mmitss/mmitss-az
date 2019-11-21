#!/bin/bash
# This script finds all the Makefiles present in the rse sources, builds that 
# project for linux and copies it to the applications folder in this directory

red='\033[0;31m'

green='\033[0;32m'
nocolor='\033[0m'

################################## COMMON APPLICATIONS ################################

#######################################################################################
echo "Cleaning Message Encoder..."
cd ./../src/common/MsgTransceiver/MsgEncoder
# Clean the folder and build for linux.
make clean &> /dev/null


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
sleep 1s
#######################################################################################

#######################################################################################
echo "Cleaning Wireless Message Decoder..."
cd ./../src/common/MsgTransceiver/MsgDecoder/WirelessMsgDecoder
# Clean the folder and build for linux.
make clean &> /dev/null

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
sleep 1s
#######################################################################################

#######################################################################################
echo "Cleaning Host BSM Decoder..."
cd ./../src/common/MsgTransceiver/MsgDecoder/HostBsmDecoder
# Clean the folder and build for linux.
make clean &> /dev/null

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
sleep 1s
#######################################################################################

################################# VEHICLE APPLICATIONS ################################

#######################################################################################
echo "Cleaning Priority Request Generator..."
cd ./../src/obu/PriorityRequestGenerator
# Clean the folder and build for linux.
make clean &> /dev/null

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
sleep 1s
#######################################################################################

############################### INTERSECTION APPLICATIONS #############################

#######################################################################################
echo "Cleaning Priority Request Server..."
cd ./../src/rsu/priority-request-server
# Clean the folder and build for linux.
make clean &> /dev/null

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
sleep 1s
#######################################################################################

#######################################################################################
echo "Cleaning Priority Solver..."
cd ./../src/rsu/priority-solver
# Clean the folder and build for linux.
make clean &> /dev/null

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
sleep 1s
#######################################################################################

#######################################################################################
echo "Cleaning Traffic Controller Interface..."
cd ./../src/rsu/traffic-control-interface
# Clean the folder and build for linux.
make clean &> /dev/null

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
sleep 1s
#######################################################################################
