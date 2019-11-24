#!/bin/bash
#############################################################################################
#                                                                                           
# NOTICE:  Copyright 2018 Arizona Board of Regents on behalf of University of Arizona.      
# All information, intellectual, and technical concepts contained herein is and shall       
# remain the proprietary information of Arizona Board of Regents and may be covered         
# by U.S. and Foreign Patents, and patents in process.  Dissemination of this information          
# or reproduction of this material is strictly forbidden unless prior written permission    
# is obtained from Arizona Board of Regents or University of Arizona.                       
#                                                                                           
# mmitss_make_clean.sh                                                                     
# Created by Niraj Altekar                                                                  
# Transportation Research Institute                                                         
# Systems and Industrial Engineering                                                        
# The University of Arizona                                                                 
#                                                                                           
# This code was develop under the supervision of Professor Larry Head                       
# in the Transportation Research Institute.                                                 
#                                                                                           
# Operational Description:                                                               
# This script cleans all executables and .o files from the source directories.
# It is recommended to run this script before committing the changes in the source to 
# version control system. This script can be run in any of (x86 or arm) architecture based 
# environments.
#############################################################################################

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

# Return back to original directory to go over the process again for another one
cd - &> /dev/null
sleep 1s
#######################################################################################
