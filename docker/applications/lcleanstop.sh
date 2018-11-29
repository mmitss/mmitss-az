#This script is for stopping all the started MMITSS processes on local machine, and delete the log files generated in the last run.
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
echo "Stopping the lstart.sh process..."
pkill -9 lstart.sh
sleep 1s
echo "Deleting the log files from the last simulation run..."
rm /nojournal/bin/log/*
sleep 1s
echo "Stopping the sleep process..."
pkill -9 sleep
ps
