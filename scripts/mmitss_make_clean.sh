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
# lmmitss-initialize.sh                                                                     
# Created by Niraj Altekar                                                                  
# Transportation Research Institute                                                         
# Systems and Industrial Engineering                                                        
# The University of Arizona                                                                 
#                                                                                           
# This code was develop under the supervision of Professor Larry Head                       
# in the Transportation Research Institute.                                                 
#                                                                                           
# Revision History:                                                                         
# Rev00: Initial Release.                                                                   
# This script configures the directory structure and libraries required by the MMITSS apps. 
# The intersection configuration files will be stored in /nojournal/bin/                    
# The log files for each simulation run will be stored in /nojournal/bin/log/               
#                                                                                           
#############################################################################################

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
# Remove the .o files to keep the folders clean
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
# Remove the .o files to keep the folders clean
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
# Remove the .o files to keep the folders clean
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
# Remove the .o files to keep the folders clean
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
# Remove the .o files to keep the folders clean
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
# Remove the .o files to keep the folders clean
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
# Remove the .o files to keep the folders clean
rm ./*.o &> /dev/null
# Return back to original directory to go over the process again for another one
cd - &> /dev/null
sleep 1s
#######################################################################################
