#This script is for stopping all the started MMITSS processes on local machine. This will NOT delete the log files generated in the previous runs.
#Maintainer: Niraj Altekar

echo "Stopping all MMITSS processes running on the local machine"
pkill -9 M_
sleep 1s
echo "Stopping the lstart.sh process. Log files from the last run will not be deleted."
pkill -9 lstart.sh
sleep 1s
echo "Stopping the sleep process"
pkill -9 sleep
ps
