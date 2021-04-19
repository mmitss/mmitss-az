import json
import time, datetime

class Logger:
    def __init__(self, consoleStatus:bool, loggingStatus:bool, intersectionName:str):
        self.consoleStatus = consoleStatus
        self.loggingStatus = loggingStatus

        if (self.loggingStatus==True):
            timestamp = str(round(time.time(),4))
            initializationTimestamp = ('{:%m%d%Y_%H%M%S}'.format(datetime.datetime.now()))
            logfileName = "/nojournal/bin/log/" + intersectionName + "_coordinationLog_" + initializationTimestamp + ".log"
            self.logFile = open(logfileName, 'w', buffering=1)
            self.logFile.write(("[{}]".format(timestamp) + " " + "Open Coordination log file for " + intersectionName +"\n"))
            
    def loggingAndConsoleDisplay(self, logString:str):
        
        timestamp = str(round(time.time(),4))
        if (self.consoleStatus==True):
            print(("\n[{}]".format(timestamp) + " " + logString))
        if (self.loggingStatus==True):
            self.logFile.write(("\n[{}]".format(timestamp) + " " + logString + "\n"))
    
    def looging(self, logString:str):
        
        timestamp = str(round(time.time(),4))
        if (self.loggingStatus==True):
            self.logFile.write(("\n[{}]".format(timestamp) + " " + logString + "\n"))  
    
    def consoleDisplay(self, consoleString:str):
        
        timestamp = str(round(time.time(),4))
        if (self.consoleStatus == True):
            print(("\n[{}]".format(timestamp) + " " + consoleString))


    def __del__(self):
        if (self.loggingStatus == True):
            self.logFile.close()

if __name__=="__main__":
    consoleStatus = True
    loggingStatus = True
    intersectionName = "speedway-mountain"
    logger = Logger(consoleStatus, loggingStatus, intersectionName)
    logger.loggingAndConsoleDisplay(logString)("Hello! This is a test output!")
    del logger


