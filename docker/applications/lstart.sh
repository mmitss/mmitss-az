#############################################################################################
#                                                                                           #
# NOTICE:  Copyright 2018 Arizona Board of Regents on behalf of University of Arizona.      #
# All information, intellectual, and technical concepts contained herein is and shall       #
# remain the proprietary information of Arizona Board of Regents and may be covered         #
# by U.S. and Foreign Patents, and patents in process.  Dissemination of this information   #       
# or reproduction of this material is strictly forbidden unless prior written permission    #
# is obtained from Arizona Board of Regents or University of Arizona.                       #
#                                                                                           #
# lstart.sh                                                                                 #
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
# This script is the starting point for all the MMITSS applications on the local machine    #
#                                                                                           #
#############################################################################################



run_in_bg () {
	$@ &> /nojournal/bin/log/${1}.log &
	sleep 2s
}

  run_in_bg ./M_PriorityRequestServer -c 2
  run_in_bg ./M_MapSpatBroadcast 127.0.0.1 127.0.0.1 0 
  run_in_bg ./M_MapSpatReceiver
  run_in_bg ./M_PriorityRequestGenerator -c 2 -l 3	
  run_in_bg ./M_PrioritySolver   
  run_in_bg ./M_TrafficControllerInterface -c 3         
  run_in_bg ./M_LongTermPlanning
  run_in_bg ./M_TrajectoryAware 20000 1 0

# Do not remove this line
while true; do sleep 1h; done
