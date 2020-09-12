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

################################# VEHICLE APPLICATIONS ################################

#######################################################################################
echo "Building Priority Request Generator..."
cd ./../src/obu/priority-request-generator
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

# ############################### INTERSECTION APPLICATIONS #############################

#######################################################################################
echo "Building Priority Request Server..."
cd ./../src/mrp/priority-request-server
# Clean the folder and build for linux.
make clean &> /dev/null
make linux ARM=1 &> /dev/null
# Indicate Success/Failure of the build
if [ "$?" -eq "0" ]; then
    mv M_PriorityRequestServer ../../../bin/PriorityRequestServer/arm
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
echo "Building Priority Solver..."
cd ./../src/mrp/priority-request-solver
# Clean the folder and build for linux.
make clean &> /dev/null
make linux ARM=1 &> /dev/null
# Indicate Success/Failure of the build
if [ "$?" -eq "0" ]; then
    mv M_PriorityRequestSolver ../../../bin/PriorityRequestSolver/arm
	echo -e "${green}Successful${nocolor}"
else
	echo -e "${red}Failed${nocolor}"
fi
# Remove the .o files to keep the folders clean
rm ./*.o &> /dev/null
# Return back to original directory to go over the process again for another one
cd - &> /dev/null
sleep 1s
######################################################################################

#######################################################################################
echo "Building Snmp Engine..."
cd ./../src/mrp/snmp-engine
# Clean the folder and build for linux.
make clean &> /dev/null
make linux ARM=1 &> /dev/null
# Indicate Success/Failure of the build
if [ "$?" -eq "0" ]; then
    mv M_SnmpEngine ../../../bin/SnmpEngine/arm
	echo -e "${green}Successful${nocolor}"
else
	echo -e "${red}Failed${nocolor}"
fi
# Remove the .o files to keep the folders clean
rm ./*.o &> /dev/null
# Return back to original directory to go over the process again for another one
cd - &> /dev/null
sleep 1s
######################################################################################

#######################################################################################
echo "Building Traffic Controller Interface..."
cd ./../src/mrp/traffic-controller-interface
# Clean the folder and build for linux.
pyinstaller --hidden-import=pkg_resources.py2_warn --onefile --windowed M_TrafficControllerInterface.py  &> /dev/null
# Indicate Success/Failure of the build
if [ "$?" -eq "0" ]; then
    mv dist/M_TrafficControllerInterface  ../../../bin/TrafficControllerInterface/arm
	echo -e "${green}Successful${nocolor}"
else
	echo -e "${red}Failed${nocolor}"
fi
# Remove the .o files to keep the folders clean
rm -r build dist __pychache__ *.spec &> /dev/null
rm -r __pycache__ &> /dev/null
# Return back to original directory to go over the process again for another one
cd - &> /dev/null
sleep 1s
#######################################################################################

#######################################################################################
echo "Building Map Spat Broadcaster..."
cd ./../src/mrp/map-spat-broadcaster
# Clean the folder and build for linux.
pyinstaller --hidden-import=pkg_resources.py2_warn --onefile --windowed M_MapSpatBroadcaster.py  &> /dev/null
# Indicate Success/Failure of the build
if [ "$?" -eq "0" ]; then
    mv dist/M_MapSpatBroadcaster  ../../../bin/MapSpatBroadcaster/arm
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
echo "Building TrajectoryAware..."
cd ./../src/mrp/trajectory-aware
# Clean the folder and build for linux.
pyinstaller --hidden-import=pkg_resources.py2_warn --onefile --windowed M_TrajectoryAware.py  &> /dev/null
# Indicate Success/Failure of the build
if [ "$?" -eq "0" ]; then
    mv dist/M_TrajectoryAware  ../../../bin/TrajectoryAware/arm
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
echo "Building Map Engine..."
cd ./../src/mrp/map-engine
# Clean the folder and build for linux.
make clean &> /dev/null
make linux ARM=1 &> /dev/null
# Indicate Success/Failure of the build
if [ "$?" -eq "0" ]; then
    mv M_MapEngine ../../../bin/MapEngine/arm
	echo -e "${green}Successful${nocolor}"
else
	echo -e "${red}Failed${nocolor}"
fi
# Remove the .o files to keep the folders clean
rm ./*.o &> /dev/null
# Return back to original directory to go over the process again for another one
cd - &> /dev/null
sleep 1s
######################################################################################

#######################################################################################
echo "Building System Performance Data Collector..."
cd ./../src/common/system-performance-data-collector
# Clean the folder and build for linux.
pyinstaller --hidden-import=pkg_resources.py2_warn --onefile --windowed M_SystemPerformanceDataCollector.py  &> /dev/null
# Indicate Success/Failure of the build
if [ "$?" -eq "0" ]; then
    mv dist/M_SystemPerformanceDataCollector  ../../../bin/SystemPerformanceDataCollector/arm
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
echo "Building Light Siren Status Manager..."
cd ./../src/obu/light-siren-status-manager
# Clean the folder and build for linux.
pyinstaller --hidden-import=pkg_resources.py2_warn --onefile --windowed M_LightSirenStatusManager.py  &> /dev/null
# Indicate Success/Failure of the build
if [ "$?" -eq "0" ]; then
    mv dist/M_LightSirenStatusManager  ../../../bin/LightSirenStatusManager/arm
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
