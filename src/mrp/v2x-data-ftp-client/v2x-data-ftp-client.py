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

FTP_CLIENT_PASSWORD = "mmitss123"

SEC_BEFORE_RETRY = 60

def main():
    with open("/nojournal/bin/mmitss-phase3-master-config.json", 'r') as configFile:
        config = json.load(configFile)

    ftpServerIp = config["DataCollectorIP"]
    ftpServerPort = config["DataTransfer"]["FtpServerPort"]
    ftpClientUsername = config["IntersectionName"]
    ftpClientPassword = FTP_CLIENT_PASSWORD

    startHour = config["DataTransfer"]["StartTime"]["Hour"]
    startMinute = config["DataTransfer"]["StartTime"]["Minute"]
    startSecondFromMidnight = ((startHour * SEC_IN_HOUR) + (startMinute * SEC_IN_MINUTE))
    
    endHour = config["DataTransfer"]["EndTime"]["Hour"]
    endMinute = config["DataTransfer"]["EndTime"]["Minute"]
    endSecondFromMidnight = ((endHour * SEC_IN_HOUR) + (endMinute * SEC_IN_MINUTE))

    if ((endSecondFromMidnight < startSecondFromMidnight) or startHour >= HOURS_IN_DAY or endHour >= HOURS_IN_DAY or startMinute >= MINUTES_IN_HOUR or endMinute >= MINUTES_IN_HOUR):
        startSecondFromMidnight = ((DEFAULT_START_HOUR * SEC_IN_HOUR) + (DEFAULT_START_MINUTE * SEC_IN_MINUTE))
        endSecondFromMidnight = ((DEFAULT_END_HOUR * SEC_IN_HOUR) + (DEFAULT_END_MINUTE * SEC_IN_MINUTE))

    maxRetries = config["DataTransfer"]["MaxRetries"]

    archivePath = '/nojournal/bin/v2x-data/archive'

    attempt = 0

    while True:
        now = datetime.datetime.now()
        nowSecondFromMidnight = ((now.hour * SEC_IN_HOUR) + (now.minute * SEC_IN_MINUTE) + (now.second))
        
        if nowSecondFromMidnight < startSecondFromMidnight:
            sleepTime = startSecondFromMidnight - nowSecondFromMidnight
            time.sleep(sleepTime)

        elif nowSecondFromMidnight >= endSecondFromMidnight:
            sleepTime = SEC_IN_DAY - nowSecondFromMidnight + startSecondFromMidnight
            time.sleep(sleepTime)

        else: 
            while (nowSecondFromMidnight >= startSecondFromMidnight and nowSecondFromMidnight < endSecondFromMidnight):
                now = datetime.datetime.now()
                nowSecondFromMidnight = ((now.hour * SEC_IN_HOUR) + (now.minute * SEC_IN_MINUTE) + (now.second))
                if attempt < maxRetries:
                    attempt = attempt + 1
                    serverIsReachable = check_network_connection(ftpServerIp)
                    if (serverIsReachable):
                        ftpc = V2XDataFtpClient(ftpServerIp, ftpServerPort, ftpClientUsername, ftpClientPassword)
                        archivedDirectories = list(os.walk(archivePath))[0][1]
                        if len(archivedDirectories) > 0:
                            for directory in archivedDirectories:
                                directoryPath = archivePath + "/" + directory
                                filenames = os.listdir((directoryPath))
                                for filename in filenames:
                                    ftpc.transfer_data(directoryPath, filename)
                                filenames = os.listdir((directoryPath))
                                if len(filenames) == 0:
                                    os.rmdir(directoryPath)
                                    archivedDirectories = list(os.walk(archivePath))[0][1]
                            print("[" + str(datetime.datetime.now()) + "]" + " Data transfer is successful: Attempt#" + str(attempt))
                            attempt = maxRetries
                            ftpc.close_connection()
                            break
                        else:
                            print("[" + str(datetime.datetime.now()) + "]" + " No files to transfer")
                            time.sleep(endSecondFromMidnight-nowSecondFromMidnight)
                    else: 
                        print("[" + str(datetime.datetime.now()) + "]" + " Server not reachable - will try again after 60 seconds: Attempt#" + str(attempt) + " failed!")
                        time.sleep(SEC_BEFORE_RETRY)
                
                else: 
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

