#!/bin/bash
#############################################################################################
#                                                                                           #
# NOTICE:  Copyright 2018 Arizona Board of Regents on behalf of University of Arizona.      #
# All information, intellectual, and technical concepts contained herein is and shall       #
# remain the proprietary information of Arizona Board of Regents and may be covered         #
# by U.S. and Foreign Patents, and patents in process.  Dissemination of this information   #       
# or reproduction of this material is strictly forbidden unless prior written permission    #
# is obtained from Arizona Board of Regents or University of Arizona.                       #
#                                                                                           #
# lmmitss-initialize.sh                                                                     #
# Created by Niraj Altekar                                                                  #
# Transportation Research Institute                                                         #
# Systems and Industrial Engineering                                                        #
# The University of Arizona                                                                 #
#                                                                                           #
# This code was develop under the supervision of Professor Larry Head                       #
# in the Transportation Research Institute.                                                 #
#                                                                                           #
# Revision History:                                                                         #
# Rev00: Initial Release.                                                                   #
# This script configures the directory structure and libraries required by the MMITSS apps. #
# The intersection configuration files will be stored in /nojournal/bin/                    #
# The log files for each simulation run will be stored in /nojournal/bin/log/               #
#                                                                                           #
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
echo "Starting Message Sender..."
cd ./../src/common/MsgTransceiver/MsgSender
python3 M_MsgSender.py > /dev/null 2>&1 &

# Return back to original directory to go over the process again for another one
cd - &> /dev/null
sleep 1s
#######################################################################################

#######################################################################################
echo "Starting Wireless Message Receiver..."
cd ./../src/common/MsgTransceiver/MsgReceiver
python3 M_WirelessMsgReceiver.py > /dev/null 2>&1 &

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

############################### INTERSECTION APPLICATIONS #############################

#######################################################################################
echo "Starting Map Spat Broadcaster"
cd ./../src/mrp/map-spat-broadcaster
python3 M_MapSpatBroadcaster.py > /dev/null 2>&1 &

# Return back to original directory to go over the process again for another one
cd - &> /dev/null
sleep 1s
#######################################################################################

#######################################################################################
echo "Starting Priority Request Server..."
cd ./../src/rsu/priority-request-server
./M_PriorityRequestServer > /dev/null 2>&1 &

# Return back to original directory to go over the process again for another one
cd - &> /dev/null
sleep 1s
#######################################################################################

#######################################################################################
echo "Starting Priority Solver..."
cd ./../src/rsu/priority-solver
./M_PrioritySolver > /dev/null 2>&1 &

# Return back to original directory to go over the process again for another one
cd - &> /dev/null
sleep 1s
#######################################################################################

#######################################################################################
echo "Starting Traffic Controller Interface..."
cd ./../src/rsu/traffic-control-interface
./M_TrafficControllerInterface > /dev/null 2>&1 &

# Return back to original directory to go over the process again for another one
cd - &> /dev/null
sleep 1s
#######################################################################################