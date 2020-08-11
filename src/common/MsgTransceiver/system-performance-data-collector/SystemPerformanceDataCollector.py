import time
import datetime
import csv

class SystemPerformanceDataCollector:
    def __init__(self,applicationPlatform):
        if applicationPlatform == "vehicle":
            vehicleLogFileName = open("/nojournal/bin/log/mmitss-system-performance-data-vehicleLog.csv", 'w')
            fields = ["MsgType","MsgCount","LoggingTime", "LoggingDateTime"]
            csvwriter = csv.writer(vehicleLogFileName)
            csvwriter.writerow(fields)
            vehicleLogFileName.close()
            # hostBSMLogFileName = open("/nojournal/bin/log/system-performance-data-hostBSMLog.csv", 'w')
            # fields = ["MsgType","MsgCount","LoggingTime"]
            # csvwriter = csv.writer(hostBSMLogFileName)
            # csvwriter.writerow(fields)
            # hostBSMLogFileName.close()
        elif applicationPlatform == "roadside":
            roadsideLogFileName = open("/nojournal/bin/log/system-performance-data-roadsideLog.csv", 'w')
            fields = ["MsgType","MsgCount","LoggingTime", "LoggingDateTime"]
            csvwriter = csv.writer(roadsideLogFileName)
            csvwriter.writerow(fields)
            roadsideLogFileName.close()
        
    def vehicleLog(self, receivedMsg):
        fileName = "/nojournal/bin/log/mmitss-system-performance-data-vehicleLog.csv"
        
        # dataLog = open(logFileName,'a+')
        msgType = receivedMsg["MsgType"]
        msgCount = receivedMsg["MsgCount"]
        loggingTime = time.time()
        loggingDateTime = datetime.datetime.now()
        dataList = [msgType, msgCount, loggingTime, loggingDateTime]
        with open(fileName, 'a+') as csvfile:
            csvwriter = csv.writer(csvfile)
            csvwriter.writerow(dataList)
            csvfile.close()
        
    def roadsideLog(self, receivedMsg):
        fileName = open("/nojournal/bin/log/mmitss-system-performance-data-roadsideLog.csv", 'a+')
        
      
        msgType = receivedMsg["MsgType"]
        msgCount = receivedMsg["MsgCount"]
        loggingTime = time.time()
        loggingDateTime = datetime.datetime.now()
        dataList = [msgType, msgCount, loggingTime, loggingDateTime]
        with open(fileName, 'a+') as csvfile:
            csvwriter = csv.writer(csvfile)
            csvwriter.writerow(dataList)
            csvfile.close()