import time
import datetime
import csv

class SystemPerformanceDataCollector:
    def __init__(self,applicationPlatform):        
        if applicationPlatform == "vehicle":
            vehicleLogFileName = open("/nojournal/bin/log/mmitss-system-performance-vehicleside-log-data.csv", 'w')
            fields = ["Message Source", "Message Type", "Message Count", "Time Interval", "Message Sent Time", "Message Log Time", "Message Logging Date & Time"]
            csvwriter = csv.writer(vehicleLogFileName)
            csvwriter.writerow(fields)
            vehicleLogFileName.close()

        elif applicationPlatform == "roadside":
            roadsideLogFileName = open("/nojournal/bin/log/mmitss-system-performance-roadside-log-data.csv", 'w')
            fields = ["Message Source", "Message Type", "Message Count", "Message Served", "Message Rejected", "Time Interval", "Message Sent Time", "Message Log Time", "Message Logging Date & Time"]
            csvwriter = csv.writer(roadsideLogFileName)
            csvwriter.writerow(fields)
            roadsideLogFileName.close()
        
    def loggingVehicleSideData(self, receivedMsg, logfile):
        msgSource = receivedMsg["MsgInformation"]["MsgSource"]
        msgType = receivedMsg["MsgInformation"]["MsgCountType"]
        msgCount = receivedMsg["MsgInformation"]["MsgCount"]
        timeInterval = receivedMsg["MsgInformation"]["TimeInterval"]
        msgSendingTime = receivedMsg["MsgInformation"]["MsgSentTime"]
        msgLoggingTime = time.time()
        msgLoggingDateTime = datetime.datetime.now()
        csvRow = (str(msgSource) + "," +
               str(msgType) + "," +
               str(msgCount) + "," +
               str(timeInterval) + "," +
               str(msgSendingTime) + "," +
               str(msgLoggingTime) + "," +
               str(msgLoggingDateTime) + "\n")
        
        logfile.write(csvRow)
                           
        
        
    def loggingRoadSideData(self, receivedMsg, logfile):        
        msgSource = receivedMsg["MsgInformation"]["MsgSource"]
        msgType = receivedMsg["MsgInformation"]["MsgCountType"]
        msgCount = receivedMsg["MsgInformation"]["MsgCount"]
        msgServed = receivedMsg["MsgInformation"]["MsgServed"]
        msgRejected = receivedMsg["MsgInformation"]["MsgRejected"]
        timeInterval = receivedMsg["MsgInformation"]["TimeInterval"]
        msgSendingTime = receivedMsg["MsgInformation"]["MsgSentTime"]
        msgLoggingTime = time.time()
        msgLoggingDateTime = datetime.datetime.now()
        
        csvRow = (str(msgSource) + "," +
                 str(msgType) + "," +
                 str(msgCount) + "," +
                 str(msgServed) + "," +
                 str(msgRejected) + "," +
                 str(timeInterval) + "," +
                 str(msgSendingTime) + "," +
                 str(msgLoggingTime) + "," +
                 str(msgLoggingDateTime) + "\n")
        
        logfile.write(csvRow)        

