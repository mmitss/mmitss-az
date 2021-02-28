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
# build.sh                                                                     
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

read -p "Build all applications? (y or n): " all
if [ "$all" = "n" ]; then

read -p "Build Transceiver applications? (y or n): " common
read -p "Build MRP applications? (y or n): " mrp
read -p "Build VSP applications? (y or n): " vsp
read -p "Build Simulation/Server applications? (y or n): " server

else
common=y
mrp=y
vsp=y
server=y
fi

######################################################################################

################################## COMMON APPLICATIONS ###############################


if [ "$common" = "y" ]; then
	echo "Building common applications..."
	######################################################################################

	echo "Building Message Encoder..."
	cd ../../src/common/MsgTransceiver/MsgEncoder
	# Clean the folder and build for linux.
	make clean &> /dev/null

	if [ "$PROCESSOR" = "arm" ]; then
		make linux ARM=1 &> /dev/null
	else
		make linux &> /dev/null
	fi

	# Indicate Success/Failure of the build
	if [ "$?" -eq "0" ]; then
		mv M_MsgEncoder ../../../../build/bin/MsgEncoder/$PROCESSOR/M_MsgEncoder
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
	cd ../../src/common/MsgTransceiver/MsgDecoder/WirelessMsgDecoder
	# Clean the folder and build for linux.
	make clean &> /dev/null

	if [ "$PROCESSOR" = "arm" ]; then
		make linux ARM=1 &> /dev/null
	else
		make linux &> /dev/null
	fi

	# Indicate Success/Failure of the build
	if [ "$?" -eq "0" ]; then
		mv M_WirelessMsgDecoder ../../../../../build/bin/WirelessMsgDecoder/$PROCESSOR/M_WirelessMsgDecoder
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
	cd ../../src/common/v2x-data-collector
	# Clean the folder and build for linux.
	pyinstaller --hidden-import=pkg_resources.py2_warn --onefile --windowed v2x-data-collector-main.py  &> /dev/null
	# Indicate Success/Failure of the build
	if [ "$?" -eq "0" ]; then
		mv dist/v2x-data-collector-main  ../../../build/bin/V2XDataCollector/$PROCESSOR/M_V2XDataCollector
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
	cd ../../src/system-interface
	# Clean the folder and build for linux.
	pyinstaller --add-data "templates:templates" --add-data "static:static" --additional-hooks-dir=. --onefile --windowed system-interface.py &> /dev/null
	# Indicate Success/Failure of the build
	if [ "$?" -eq "0" ]; then
		mv dist/system-interface  ../../build/bin/SystemInterface/$PROCESSOR/M_SystemInterface
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
fi

############################### INTERSECTION APPLICATIONS #############################
if [ "$mrp" = "y" ]; then
	echo "Building MRP applications..."

	#######################################################################################
	echo "Building Priority Request Server..."
	cd ../../src/mrp/priority-request-server
	# Clean the folder and build for linux.
	make clean &> /dev/null

	if [ "$PROCESSOR" = "arm" ]; then
		make linux ARM=1 &> /dev/null
	else
		make linux &> /dev/null
	fi

	# Indicate Success/Failure of the build
	if [ "$?" -eq "0" ]; then
		mv M_PriorityRequestServer ../../../build/bin/PriorityRequestServer/$PROCESSOR/M_PriorityRequestServer
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
	cd ../../src/mrp/priority-request-solver
	# Clean the folder and build for linux.
	make clean &> /dev/null

	if [ "$PROCESSOR" = "arm" ]; then
		make linux ARM=1 &> /dev/null
	else
		make linux &> /dev/null
	fi

	# Indicate Success/Failure of the build
	if [ "$?" -eq "0" ]; then
		mv M_PriorityRequestSolver ../../../build/bin/PriorityRequestSolver/$PROCESSOR/M_PriorityRequestSolver
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
	cd ../../src/mrp/snmp-engine
	# Clean the folder and build for linux.
	make clean &> /dev/null

	if [ "$PROCESSOR" = "arm" ]; then
		make linux ARM=1 &> /dev/null
	else
		make linux &> /dev/null
	fi

	# Indicate Success/Failure of the build
	if [ "$?" -eq "0" ]; then
		mv M_SnmpEngine ../../../build/bin/SnmpEngine/$PROCESSOR/M_SnmpEngine
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
	cd ../../src/mrp/traffic-controller-interface
	# Clean the folder and build for linux.
	pyinstaller --hidden-import=pkg_resources.py2_warn --onefile --windowed traffic-controller-interface.py  &> /dev/null
	# Indicate Success/Failure of the build
	if [ "$?" -eq "0" ]; then
		mv dist/traffic-controller-interface  ../../../build/bin/TrafficControllerInterface/$PROCESSOR/M_TrafficControllerInterface
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
	cd ../../src/mrp/map-spat-broadcaster
	# Clean the folder and build for linux.
	pyinstaller --hidden-import=pkg_resources.py2_warn --onefile --windowed map-spat-broadcaster.py  &> /dev/null
	# Indicate Success/Failure of the build
	if [ "$?" -eq "0" ]; then
		mv dist/map-spat-broadcaster  ../../../build/bin/MapSpatBroadcaster/$PROCESSOR/M_MapSpatBroadcaster
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
	cd ../../src/mrp/signal-coordination-request-generator
	# Clean the folder and build for linux.
	pyinstaller --hidden-import=pkg_resources.py2_warn --onefile --windowed signal-coordination-request-generator.py  &> /dev/null
	# Indicate Success/Failure of the build
	if [ "$?" -eq "0" ]; then
		mv dist/signal-coordination-request-generator  ../../../build/bin/SignalCoordinationRequestGenerator/$PROCESSOR/M_SignalCoordinationRequestGenerator
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
	echo "Building V2X Data Transfer.."
	cd ../../src/common/v2x-data-transfer
	# Clean the folder and build for linux.
	pyinstaller --hidden-import=pkg_resources.py2_warn --onefile --windowed v2x-data-transfer-main.py  &> /dev/null
	# Indicate Success/Failure of the build
	if [ "$?" -eq "0" ]; then
		mv dist/v2x-data-transfer-main  ../../../build/bin/V2XDataTransfer/$PROCESSOR/M_V2XDataTransfer
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
fi

################################# VEHICLE APPLICATIONS ################################

if [ "$vsp" = "y" ]; then
	echo "Building VSP applications..."

	#######################################################################################
	echo "Building Host BSM Decoder..."
	cd ../../src/common/MsgTransceiver/MsgDecoder/HostBsmDecoder
	# Clean the folder and build for linux.
	make clean &> /dev/null

	if [ "$PROCESSOR" = "arm" ]; then
		make linux ARM=1 &> /dev/null
	else
		make linux &> /dev/null
	fi

	# Indicate Success/Failure of the build
	if [ "$?" -eq "0" ]; then
		mv M_HostBsmDecoder ../../../../../build/bin/HostBsmDecoder/$PROCESSOR/M_HostBsmDecoder
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
	cd ../../src/vsp/priority-request-generator
	# Clean the folder and build for linux.
	make clean &> /dev/null

	if [ "$PROCESSOR" = "arm" ]; then
		make linux ARM=1 &> /dev/null
	else
		make linux &> /dev/null
	fi

	# Indicate Success/Failure of the build
	if [ "$?" -eq "0" ]; then
		mv M_PriorityRequestGenerator ../../../build/bin/PriorityRequestGenerator/$PROCESSOR/M_PriorityRequestGenerator
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
	cd ../../src/vsp/light-siren-status-manager
	# Clean the folder and build for linux.
	pyinstaller --hidden-import=pkg_resources.py2_warn --onefile --windowed light-siren-status-manager.py  &> /dev/null
	# Indicate Success/Failure of the build
	if [ "$?" -eq "0" ]; then
		mv dist/light-siren-status-manager  ../../../build/bin/LightSirenStatusManager/$PROCESSOR/M_LightSirenStatusManager
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
	echo "Building Data Compressor..."
	cd ../../src/vsp/data-compressor
	# Clean the folder and build for linux.
	pyinstaller --hidden-import=pkg_resources.py2_warn --onefile --windowed data-compressor.py  &> /dev/null
	# Indicate Success/Failure of the build
	if [ "$?" -eq "0" ]; then
		mv dist/data-compressor  ../../../build/bin/DataCompressor/$PROCESSOR/M_DataCompressor
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
fi

################################### SERVER TOOLS ##################################

if [ "$server" = "y" ]; then
	echo "Building server-tools..."


	#######################################################################################
	echo "Building Priority Request Generator Server..."
	cd ../../src/server/priority-request-generator-server
	# Clean the folder and build for linux.
	make clean &> /dev/null

	if [ "$PROCESSOR" = "arm" ]; then
		make linux ARM=1 &> /dev/null
	else
		make linux &> /dev/null
	fi

	# Indicate Success/Failure of the build
	if [ "$?" -eq "0" ]; then
		mv M_PriorityRequestGeneratorServer ../../../build/bin/PriorityRequestGeneratorServer/$PROCESSOR/M_PriorityRequestGeneratorServer
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
	cd ../../src/server/message-distributor
	# Clean the folder and build for linux.
	pyinstaller --hidden-import=pkg_resources.py2_warn --onefile --windowed message-distributor.py  &> /dev/null
	# Indicate Success/Failure of the build
	if [ "$?" -eq "0" ]; then
		mv dist/message-distributor  ../../../build/bin/MessageDistributor/$PROCESSOR/M_MessageDistributor
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
	cd ../../src/simulation/simulated-bsm-blob-processor
	# Clean the folder and build for linux.
	pyinstaller --hidden-import=pkg_resources.py2_warn --onefile --windowed simulated-bsm-blob-processor.py  &> /dev/null
	# Indicate Success/Failure of the build
	if [ "$?" -eq "0" ]; then
		mv dist/simulated-bsm-blob-processor  ../../../build/bin/SimulatedBsmBlobProcessor/$PROCESSOR/M_SimulatedBsmBlobProcessor
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
fi


