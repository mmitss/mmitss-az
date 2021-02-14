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
# build-x86.sh                                                                     
# Created by Niraj Altekar                                                                  
# Transportation Research Institute                                                         
# Systems and Industrial Engineering                                                        
# The University of Arizona                                                                 
#                                                                                           
# This code was develop under the supervision of Professor Larry Head                       
# in the Transportation Research Institute.                                                 
#                                                                                           
# Operational Description:                                                                   
# This script builds all mmitss applications (vehicle, intersection, common, and simulation),
# under the x86 environment. The primary reason for such builds is development and testing.
# This script can not be used in the ARM architecture based devices.                                                                                                  
#############################################################################################

# Define colors:
red='\033[0;31m'
green='\033[0;32m'
nocolor='\033[0m'

################################## COMMON APPLICATIONS ################################

#######################################################################################
echo "Building V2X Data Collector..."
cd ./../src/common/v2x-data-collector
# Clean the folder and build for linux.
pyinstaller --hidden-import=pkg_resources.py2_warn --onefile --windowed v2x-data-collector-main.py  &> /dev/null
# Indicate Success/Failure of the build
if [ "$?" -eq "0" ]; then
    mv dist/v2x-data-collector-main  ../../../bin/V2XDataCollector/x86/M_V2XDataCollector
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

############################### INTERSECTION APPLICATIONS #############################

#######################################################################################
echo "Building Priority Request Server..."
cd ./../src/mrp/priority-request-server
# Clean the folder and build for linux.
make clean &> /dev/null
make linux &> /dev/null
# Indicate Success/Failure of the build
if [ "$?" -eq "0" ]; then
    mv M_PriorityRequestServer ../../../bin/PriorityRequestServer/x86
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
make linux &> /dev/null
# Indicate Success/Failure of the build
if [ "$?" -eq "0" ]; then
    mv M_PriorityRequestSolver ../../../bin/PriorityRequestSolver/x86
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
make linux &> /dev/null
# Indicate Success/Failure of the build
if [ "$?" -eq "0" ]; then
    mv M_SnmpEngine ../../../bin/SnmpEngine/x86
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
pyinstaller --hidden-import=pkg_resources.py2_warn --onefile --windowed traffic-controller-interface.py  &> /dev/null
# Indicate Success/Failure of the build
if [ "$?" -eq "0" ]; then
    mv dist/traffic-controller-interface  ../../../bin/TrafficControllerInterface/x86/M_TrafficControllerInterface
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
pyinstaller --hidden-import=pkg_resources.py2_warn --onefile --windowed map-spat-broadcaster.py  &> /dev/null
# Indicate Success/Failure of the build
if [ "$?" -eq "0" ]; then
    mv dist/map-spat-broadcaster  ../../../bin/MapSpatBroadcaster/x86/M_MapSpatBroadcaster
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
echo "Building V2X Data Ftp Client..."
cd ./../src/mrp/v2x-data-ftp-client
# Clean the folder and build for linux.
pyinstaller --hidden-import=pkg_resources.py2_warn --onefile --windowed v2x-data-ftp-client-main.py  &> /dev/null
# Indicate Success/Failure of the build
if [ "$?" -eq "0" ]; then
    mv dist/v2x-data-ftp-client-main  ../../../bin/V2XDataFtpClient/x86/M_V2XDataFtpClient
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

################################### SIMULATION TOOLS ##################################

#######################################################################################
echo "Building Priority Request Generator Server..."
cd ./../src/simulation/priority-request-generator-server
# Clean the folder and build for linux.
make clean &> /dev/null
make linux &> /dev/null
# Indicate Success/Failure of the build
if [ "$?" -eq "0" ]; then
    mv M_PriorityRequestGeneratorServer ../../../bin/PriorityRequestGeneratorServer/x86
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
echo "Building Message Distributor..."
cd ./../src/simulation/message-distributor
# Clean the folder and build for linux.
pyinstaller --hidden-import=pkg_resources.py2_warn --onefile --windowed message-distributor.py  &> /dev/null
# Indicate Success/Failure of the build
if [ "$?" -eq "0" ]; then
    mv dist/message-distributor  ../../../bin/MessageDistributor/x86/M_MessageDistributor
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
echo "Building Simulated BSM Blob Processor..."
cd ./../src/simulation/mmitss-driver-model/simulated-bsm-blob-processor
# Clean the folder and build for linux.
pyinstaller --hidden-import=pkg_resources.py2_warn --onefile --windowed simulated-bsm-blob-processor.py  &> /dev/null
# Indicate Success/Failure of the build
if [ "$?" -eq "0" ]; then
    mv dist/simulated-bsm-blob-processor  ../../../bin/MessageDistributor/x86/M_SimulatedBsmBlobProcessor
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
echo "Building Signal Coordination Request Generator..."
cd ./../src/mrp/signal-coordination-request-generator
# Clean the folder and build for linux.
pyinstaller --hidden-import=pkg_resources.py2_warn --onefile --windowed signal-coordination-request-generator.py  &> /dev/null
# Indicate Success/Failure of the build
if [ "$?" -eq "0" ]; then
    mv dist/signal-coordination-request-generator  ../../../bin/SignalCoordinationRequestGenerator/x86/M_SignalCoordinationRequestGenerator
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
