# This script is the starting point for all the MMITSS applications on the local machine
# Maintainer: Niraj Altekar

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
