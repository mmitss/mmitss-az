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
# build-arm.sh                                                                     
# Created by Niraj Altekar                                                                  
# Transportation Research Institute                                                         
# Systems and Industrial Engineering                                                        
# The University of Arizona                                                                 
#                                                                                           
# This code was develop under the supervision of Professor Larry Head                       
# in the Transportation Research Institute.                                                 
#                                                                                           
# Operational Description:                                                                   
# This script builds all mmitss applications (vehicle, intersection, and common),
# under the arm environment. The primary reason for such builds is development and testing.
# This script can not be used in the x86 architecture based devices.                                                                                                  
#############################################################################################

# Define colors:
red='\033[0;31m'
green='\033[0;32m'
nocolor='\033[0m'

######################################################################################

################################## COMMON APPLICATIONS ################################

######################################################################################
echo "Building Message Encoder..."
cd ./../src/common/MsgTransceiver/MsgEncoder
# Clean the folder and build for linux.
make clean &> /dev/null
make linux ARM=1 &> /dev/null

# Indicate Success/Failure of the build
if [ "$?" -eq "0" ]; then
    mv M_MsgEncoder ../../../../bin/MsgEncoder/arm
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
echo "Building Wireless Message Decoder..."
cd ./../src/common/MsgTransceiver/MsgDecoder/WirelessMsgDecoder
# Clean the folder and build for linux.
make clean &> /dev/null
make linux ARM=1 &> /dev/null
# Indicate Success/Failure of the build
if [ "$?" -eq "0" ]; then
    mv M_WirelessMsgDecoder ../../../../../bin/WirelessMsgDecoder/arm
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
echo "Building V2X Data Collector..."
cd ./../src/common/v2x-data-collector
# Clean the folder and build for linux.
pyinstaller --hidden-import=pkg_resources.py2_warn --onefile --windowed v2x-data-collector.py  &> /dev/null
# Indicate Success/Failure of the build
if [ "$?" -eq "0" ]; then
    mv dist/v2x-data-collector  ../../../bin/V2XDataCollector/arm/M_V2XDataCollector
	echo -e "${green}Successful${nocolor}"
else
	echo -e "${red}Failed${nocolor}"
fi
# Remove the .o files to keep the folders clean
rm -r build dist *.spec &> /dev/null
rm -r __pycache__ &> /dev/null
# Return back to original directory to go over the process again for another one
cd - &> /dev/null
sleep 1s
#######################################################################################

#######################################################################################
echo "Building System Interface..."
cd ./../src/system-interface
# Clean the folder and build for linux.
pyinstaller --add-data "templates:templates" --add-data "static:static" --additional-hooks-dir=. --onefile --windowed system-interface.py &> /dev/null
# Indicate Success/Failure of the build
if [ "$?" -eq "0" ]; then
    mv dist/system-interface  ../../bin/SystemInterface/arm/M_SystemInterface
	echo -e "${green}Successful${nocolor}"
else
	echo -e "${red}Failed${nocolor}"
fi
# Remove the files to keep the folders clean
rm -r build dist *.spec &> /dev/null
rm -r __pycache__ &> /dev/null
# Return back to original directory to go over the process again for another one
cd - &> /dev/null
sleep 1s
#######################################################################################

################################# VEHICLE APPLICATIONS ################################

#######################################################################################
echo "Building Host BSM Decoder..."
cd ./../src/common/MsgTransceiver/MsgDecoder/HostBsmDecoder
# Clean the folder and build for linux.
make clean &> /dev/null
make linux ARM=1 &> /dev/null
# Indicate Success/Failure of the build
if [ "$?" -eq "0" ]; then
    mv M_HostBsmDecoder ../../../../../bin/HostBsmDecoder/arm
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
echo "Building Priority Request Generator..."
cd ./../src/vsp/priority-request-generator
# Clean the folder and build for linux.
make clean &> /dev/null
make linux ARM=1 &> /dev/null
# Indicate Success/Failure of the build
if [ "$?" -eq "0" ]; then
    mv M_PriorityRequestGenerator ../../../bin/PriorityRequestGenerator/arm
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
echo "Building Light Siren Status Manager..."
cd ./../src/vsp/light-siren-status-manager
# Clean the folder and build for linux.
pyinstaller --hidden-import=pkg_resources.py2_warn --onefile --windowed light-siren-status-manager.py  &> /dev/null
# Indicate Success/Failure of the build
if [ "$?" -eq "0" ]; then
    mv dist/light-siren-status-manager  ../../../bin/LightSirenStatusManager/arm/M_LightSirenStatusManager
	echo -e "${green}Successful${nocolor}"
else
	echo -e "${red}Failed${nocolor}"
fi
# Remove the .o files to keep the folders clean
rm -r build dist *.spec &> /dev/null
rm -r __pycache__ &> /dev/null
# Return back to original directory to go over the process again for another one
cd - &> /dev/null
sleep 1s
#######################################################################################

