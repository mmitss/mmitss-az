import json
import sys
import time

from PullFromIntersection import PullFromIntersection
from PushToCyverse import PushToCyverse
from PushToServer import PushToServer
from Scheduler import Scheduler

DEBUGGING_ROADSIDE = False
DEBUGGING_SERVER = True

def get_roadside_config(config:dict):
    try:
        server = config["DataTransfer"]["server"]
        intersection = config["DataTransfer"]["intersection"]
        startHour = config["DataTransfer"]["StartTime"]["hour"]
        startMinute = config["DataTransfer"]["StartTime"]["minute"]
        return server, intersection, startHour, startMinute
    except Exception as e:
        print(e)
        return -1, -1, -1, -1

def get_server_pull_config(config:dict):
    try:
        server = config["DataTransfer"]["server"]
        intersection = config["DataTransfer"]["intersection"]
        startHour = config["DataTransfer"]["StartTime_FromIntersections"]["hour"]
        startMinute = config["DataTransfer"]["StartTime_FromIntersections"]["minute"]
        return server, intersection, startHour, startMinute
    except Exception as e:
        print(e)
        return -1, -1, -1, -1

def get_server_cyverse_push_config(config:dict):
        try:
            server = config["DataTransfer"]["server"]
            intersection = config["DataTransfer"]["intersection"]
            startHour = config["DataTransfer"]["StartTime_ToCyverse"]["hour"]
            startMinute = config["DataTransfer"]["StartTime_ToCyverse"]["minute"]
            return server, intersection, startHour, startMinute
        except Exception as e:
            print(e)
            return -1, -1, -1, -1
def main():
    if DEBUGGING_ROADSIDE and DEBUGGING_ROADSIDE:
        print("Invalid debugging configuration.")
        exit()
    elif DEBUGGING_ROADSIDE: 
        configFilename = "test/mmitss-phase3-master-config-roadside.json"
    elif DEBUGGING_SERVER:
        configFilename = "test/mmitss-phase3-master-config-server.json"
    else: 
        configFilename = "/nojournal/bin/mmitss-phase3-master-config.json"
    
    with open(configFilename, 'r') as configFile:
        config = json.load(configFile)

    scheduler = Scheduler()
    applicationPlatform = config["ApplicationPlatform"]

    if applicationPlatform == "roadside":
        server, intersection, startHour, startMinute = get_roadside_config(config)
        if server != -1 and intersection != -1 and startHour != -2 and startMinute != -1:
            pushToServer = PushToServer(server, intersection)
            scheduler.schedule_daily_execution(pushToServer.transfer_data, startHour, startMinute)
            while True: time.sleep(3600)
        else: print("Could not schedule roadside data transfer!")
    
    elif applicationPlatform == "server":
        serverScheduled = False
        if config["PullFromIntersections"] == True:
            server, intersection, startHour, startMinute = get_server_pull_config(config)
            if server != -1 and intersection != -1 and startHour != -1 and startMinute != -1:
                pullFromIntersection = PullFromIntersection(server, intersection)
                scheduler.schedule_daily_execution(pullFromIntersection.transfer_data, startHour, startMinute)
                serverScheduled = True
            else: print("Could not schedule pull from intersection data transfer!")
        if config["PushToCyverse"] == True:
            if server != -1 and intersection != -1 and startHour != -1 and startMinute != -1:
                server, intersection, startHour, startMinute = get_server_cyverse_push_config(config)
                pushToCyverse = PushToCyverse(server, intersection)
                scheduler.schedule_daily_execution(pushToCyverse.transfer_data, startHour, startMinute)
                serverScheduled = True
            else: print("Could not schedule push to CyVerse data transfer!")
        if serverScheduled != False:
            while True: time.sleep(3600)
    else:
        print("Nothing to schedule!")
        exit()


    
    
    



if __name__=="__main__":
    main()