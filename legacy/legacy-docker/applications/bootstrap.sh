#!/bin/bash
## This script is the starting point for all the MMITSS applications inside the Docker Container

run_in_bg () {
	$@ &> /nojournal/bin/log/${1}.log &
	sleep 2s
}

# ifconfig > /nojournal/bin/log/my_ip.txt

  run_in_bg M_MapSpatBroadcast 127.0.0.1 127.0.0.1 0 
  run_in_bg M_MapSpatReceiver
  run_in_bg M_PriorityRequestGenerator -c 2 -l 3
  run_in_bg M_PriorityRequestServer -c 2	
  run_in_bg M_PrioritySolver   
  run_in_bg M_TrafficControllerInterface -c 3      

# Do not remove this line
while true; do sleep 1h; done
