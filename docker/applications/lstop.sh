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
