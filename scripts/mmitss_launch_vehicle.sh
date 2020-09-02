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
# mmitss_launch_vehicle.sh                                                                     
# Created by Niraj Altekar                                                                  
# Transportation Research Institute                                                         
# Systems and Industrial Engineering                                                        
# The University of Arizona                                                                 
#                                                                                           
# This code was develop under the supervision of Professor Larry Head                       
# in the Transportation Research Institute.                                                 
#                                                                                           
# Operational Description                                                                       
# This script launches all vehicle-side mmitss applications in the background, and 
# discards the output by redirecting it to /dev/null. This script assumes that all 
# applications are already built, and the executables are available in the source 
# directories.                                                                                                     
#############################################################################################

################################## COMMON APPLICATIONS ################################

#######################################################################################
echo "Starting Message Encoder..."
cd ./../src/common/MsgTransceiver/MsgEncoder
./M_MsgEncoder > /dev/null 2>&1 &

# Return back to original directory to go over the process again for another one
cd - &> /dev/null
sleep 1s
#######################################################################################

#######################################################################################
echo "Starting Wireless Message Decoder..."
cd ./../src/common/MsgTransceiver/MsgDecoder/WirelessMsgDecoder
./M_WirelessMsgDecoder > /dev/null 2>&1 &

# Return back to original directory to go over the process again for another one
cd - &> /dev/null
sleep 1s
#######################################################################################

#######################################################################################
echo "Starting Host BSM Decoder..."
cd ./../src/common/MsgTransceiver/MsgDecoder/HostBsmDecoder
./M_HostBsmDecoder > /dev/null 2>&1 &

# Return back to original directory to go over the process again for another one
cd - &> /dev/null
sleep 1s
#######################################################################################

################################# VEHICLE APPLICATIONS ################################

#######################################################################################
echo "Starting Priority Request Generator..."
cd ./../src/obu/priority-request-generator
./M_PriorityRequestGenerator > /dev/null 2>&1 &

# Return back to original directory to go over the process again for another one
cd - &> /dev/null
sleep 1s
#######################################################################################

#######################################################################################
echo "Starting System Performance Data Collector..."
cd ./../src/common/system-performance-data-collector
python3 M_SystemPerformanceDataCollector.py > /dev/null 2>&1 &

# Return back to original directory to go over the process again for another one
cd - &> /dev/null
sleep 1s
#######################################################################################
