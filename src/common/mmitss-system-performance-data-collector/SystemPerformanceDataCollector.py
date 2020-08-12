import time
import datetime
import csv

class SystemPerformanceDataCollector:
    def __init__(self,applicationPlatform):
        if applicationPlatform == "vehicle":
            vehicleLogFileName = open("/nojournal/bin/log/mmitss-system-performance-vehicleside-log-data.csv", 'w')
            fields = ["Message Source", "Message Type", "Message Count", "Time Interval", "Message Sending Time", "Logging Time", "Logging Date & Time"]
            csvwriter = csv.writer(vehicleLogFileName)
            csvwriter.writerow(fields)
            vehicleLogFileName.close()

        elif applicationPlatform == "roadside":
            roadsideLogFileName = open("/nojournal/bin/log/mmitss-system-performance-roadside-log-data.csv", 'w')
            fields = ["Message Source", "Message Type", "Message Count", "Message Served", "Message Rejected", "Time Interval", "Message Sending Time", "Logging Time", "Logging Date & Time"]
            csvwriter = csv.writer(roadsideLogFileName)
            csvwriter.writerow(fields)
            roadsideLogFileName.close()
        
    def loggingVehicleSideData(self, receivedMsg):
        fileName = "/nojournal/bin/log/mmitss-system-performance-vehicleside-log-data.csv"

        msgSource = receivedMsg["MsgInformation"]["MsgSource"]
        msgType = receivedMsg["MsgInformation"]["MsgCountType"]
        msgCount = receivedMsg["MsgInformation"]["MsgCount"]
        timeInterval = receivedMsg["MsgInformation"]["TimeInterval"]
        msgSendingTime = receivedMsg["MsgInformation"]["MsgSendingTime"]
        loggingTime = time.time()
        loggingDateTime = datetime.datetime.now()
        dataList = [msgSource, msgType, msgCount, timeInterval, msgSendingTime, loggingTime, loggingDateTime]
        with open(fileName, 'a+') as csvfile:
            csvwriter = csv.writer(csvfile)
            csvwriter.writerow(dataList)
            csvfile.close()
        
    def loggingRoadSideData(self, receivedMsg):
        fileName = "/nojournal/bin/log/mmitss-system-performance-roadside-log-data.csv"
        
        msgSource = receivedMsg["MsgInformation"]["MsgSource"]
        msgType = receivedMsg["MsgInformation"]["MsgCountType"]
        msgCount = receivedMsg["MsgInformation"]["MsgCount"]
        msgServed = receivedMsg["MsgInformation"]["MsgServed"]
        msgRejected = receivedMsg["MsgInformation"]["MsgRejected"]
        timeInterval = receivedMsg["MsgInformation"]["TimeInterval"]
        msgSendingTime = receivedMsg["MsgInformation"]["MsgSendingTime"]
        loggingTime = time.time()
        loggingDateTime = datetime.datetime.now()
        dataList = [msgSource, msgType, msgCount, msgServed, msgRejected, timeInterval, msgSendingTime, loggingTime, loggingDateTime]
        with open(fileName, 'a+') as csvfile:
            csvwriter = csv.writer(csvfile)
            csvwriter.writerow(dataList)
            csvfile.close()