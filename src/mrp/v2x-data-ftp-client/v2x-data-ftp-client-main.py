"""
***************************************************************************************

 © 2019 Arizona Board of Regents on behalf of the University of Arizona with rights
       granted for USDOT OSADP distribution with the Apache 2.0 open source license.

***************************************************************************************

v2x-data-ftp-client.py
Created by: Niraj Vasant Altekar
University of Arizona   
College of Engineering

This code was developed under the supervision of Professor Larry Head
in the Systems and Industrial Engineering Department.

***************************************************************************************
"""

import os
import subprocess
import json
import time, datetime
from V2XDataFtpClient import V2XDataFtpClient

HOURS_IN_DAY = 24
MINUTES_IN_HOUR = 60

SEC_IN_DAY = 24 * 60 * 60
SEC_IN_HOUR = 60 * 60
SEC_IN_MINUTE = 60

DEFAULT_START_HOUR = 1
DEFAULT_START_MINUTE = 0
DEFAULT_END_HOUR = 1
DEFAULT_END_MINUTE = 30

FTP_CLIENT_PASSWORD = "MmitssIntersection"

SEC_BEFORE_RETRY = 60

def main():
    # Read the config file
    with open("/nojournal/bin/mmitss-phase3-master-config.json", 'r') as configFile:
        config = json.load(configFile)

    # Read FTP server and own (client) information:
    ftpServerIp = config["DataCollectorIP"]
    ftpServerPort = config["DataTransfer"]["FtpServerPort"]
    ftpClientUsername = config["IntersectionName"]
    ftpClientPassword = FTP_CLIENT_PASSWORD

    # Read the start and end times of the allocated interval and convert them into secondsFromMidnight
    startHour = config["DataTransfer"]["StartTime"]["Hour"]
    startMinute = config["DataTransfer"]["StartTime"]["Minute"]
    startSecondFromMidnight = ((startHour * SEC_IN_HOUR) + (startMinute * SEC_IN_MINUTE))
    
    endHour = config["DataTransfer"]["EndTime"]["Hour"]
    endMinute = config["DataTransfer"]["EndTime"]["Minute"]
    endSecondFromMidnight = ((endHour * SEC_IN_HOUR) + (endMinute * SEC_IN_MINUTE))

    # If invalid combination of start and end times is detected, assign the default interval
    if ((endSecondFromMidnight < startSecondFromMidnight) or startHour >= HOURS_IN_DAY or endHour >= HOURS_IN_DAY or startMinute >= MINUTES_IN_HOUR or endMinute >= MINUTES_IN_HOUR):
        startSecondFromMidnight = ((DEFAULT_START_HOUR * SEC_IN_HOUR) + (DEFAULT_START_MINUTE * SEC_IN_MINUTE))
        endSecondFromMidnight = ((DEFAULT_END_HOUR * SEC_IN_HOUR) + (DEFAULT_END_MINUTE * SEC_IN_MINUTE))

    # Read maximum number of allowed attempts
    maxRetries = config["DataTransfer"]["MaxRetries"]

    # Define the path where archives are stored
    archivePath = '/nojournal/bin/v2x-data/archive'

    # Initialize the attempt variable
    attempt = 0

    while True:
        # Get current time and convert it to secondsFromMidnight
        now = datetime.datetime.now()
        nowSecondFromMidnight = ((now.hour * SEC_IN_HOUR) + (now.minute * SEC_IN_MINUTE) + (now.second))
        
        # If the allocated time interval is yet to begin, sleep until it begins
        if nowSecondFromMidnight < startSecondFromMidnight:
            sleepTime = startSecondFromMidnight - nowSecondFromMidnight
            time.sleep(sleepTime)

        # If the allocated time interval for the day is already over, sleep until the start of allocated interval on the next day
        elif nowSecondFromMidnight >= endSecondFromMidnight:
            sleepTime = SEC_IN_DAY - nowSecondFromMidnight + startSecondFromMidnight
            time.sleep(sleepTime)

        else: # if the allocated time interval is ongoing, transfer files to the FTP server.
            while (nowSecondFromMidnight >= startSecondFromMidnight and nowSecondFromMidnight < endSecondFromMidnight):
                now = datetime.datetime.now()
                nowSecondFromMidnight = ((now.hour * SEC_IN_HOUR) + (now.minute * SEC_IN_MINUTE) + (now.second))
                if attempt < maxRetries:
                    attempt = attempt + 1
                    serverIsReachable = check_network_connection(ftpServerIp)
                    if (serverIsReachable):
                        ftpc = V2XDataFtpClient(ftpServerIp, ftpServerPort, ftpClientUsername, ftpClientPassword)
                        archivedDirectories = list(os.walk(archivePath))[0][1]
                        if len(archivedDirectories) > 0: # If there are any files to tranfer, transfer files to the server
                            for directory in archivedDirectories:
                                directoryPath = archivePath + "/" + directory
                                filenames = os.listdir((directoryPath))
                                for filename in filenames:
                                    ftpc.transfer_data(directoryPath, filename)
                                filenames = os.listdir((directoryPath))
                                if len(filenames) == 0: # If there are no more files in the directoryPath, remore the directory and update archivedDirectories variable.
                                    os.rmdir(directoryPath)
                                    archivedDirectories = list(os.walk(archivePath))[0][1]
                            print("[" + str(datetime.datetime.now()) + "]" + " Data transfer is successful: Attempt#" + str(attempt))
                            attempt = maxRetries
                            ftpc.close_connection()
                            break
                        else: # If there are no files to transfer, sleep until tomorrow
                            print("[" + str(datetime.datetime.now()) + "]" + " No files to transfer")
                            time.sleep(endSecondFromMidnight-nowSecondFromMidnight)
                    else: # If server is not reachable, try again after the defined sleep period
                        print("[" + str(datetime.datetime.now()) + "]" + " Server not reachable - will try again: Attempt#" + str(attempt) + " failed!")
                        time.sleep(SEC_BEFORE_RETRY)
                
                else: # Sleep until the current interval is over
                    time.sleep(endSecondFromMidnight-nowSecondFromMidnight)
                    break
                    

def check_network_connection(ftpServerIp):
    packets = 1
    timeout = 1000
    command = ['ping', '-c', str(packets), '-w', str(timeout), ftpServerIp]
    result = subprocess.run(command, stdin=subprocess.DEVNULL, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    return result.returncode == 0

if __name__ == "__main__":
    main()

