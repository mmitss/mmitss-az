#############################################################################################
#                                                                                           #
# NOTICE:  Copyright 2018 Arizona Board of Regents on behalf of University of Arizona.      #
# All information, intellectual, and technical concepts contained herein is and shall       #
# remain the proprietary information of Arizona Board of Regents and may be covered         #
# by U.S. and Foreign Patents, and patents in process.  Dissemination of this information   #       
# or reproduction of this material is strictly forbidden unless prior written permission    #
# is obtained from Arizona Board of Regents or University of Arizona.                       #
#                                                                                           #
# lstop.sh                                                                                  #
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
# This script is for stopping all the started MMITSS processes on local machine,            # 
# but does not delete the log files generated in the last run.                              #
#                                                                                           #
#############################################################################################


#This script is for stopping all the started MMITSS processes on local machine. This will NOT delete the log files generated in the previous runs.
#Maintainer: Niraj Altekar

echo "Stopping MapSpatBroadcaster and MapSpatReceiver..."
pkill -9 M_MapSpat
sleep 1s
echo "Stopping SignalPriority applications..."
pkill -9 M_Priority
sleep 1s
echo "Stopping TrafficControlInterface and TrajectoryAware applications..."
pkill -9 M_Tra
sleep 1s
echo "Stopping LongTermPlanning application..."
pkill -9 M_
sleep 1s
echo "Stopping the lstart.sh process. Log files from the last run will not be deleted..."
pkill -9 lstart.sh
sleep 1s
echo "Stopping the sleep process..."
pkill -9 sleep
ps
