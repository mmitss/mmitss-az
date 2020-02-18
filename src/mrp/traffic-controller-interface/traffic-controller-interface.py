import json
from SignalController import SignalController
import time
import socket


vendorId = 0 # 0:Econolite

sigController = SignalController('10.12.6.17', 501, 0)
hostIp = '10.12.6.108'
tciPort = 20005
tciAddress = (hostIp, tciPort)

currPhaseListenerPort = 30000
currPhaseListenerAddress = (hostIp, currPhaseListenerPort)

s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
s.bind(tciAddress)

while True:
    
    data, requesterAddress = s.recvfrom(512)


    receivedMsg = json.loads(data.decode())
    if(receivedMsg["MsgType"]=="RequestCurr_NextPhases"):
        

        sigController.sendCurrentAndNextPhasesDict(currPhaseListenerAddress, requesterAddress)
        #print(curr_nextPhasesJson)
    
    time.sleep(0.5)
s.close()


#print(sigController.getActiveTimingPlan())

# command1 = sigController.addCommandToSchedule(6,255,5)
# print(command1)
# time.sleep(1)
# command2 = sigController.addCommandToSchedule(7,255,6)
# print(command2)
# sigController.removeCommandFromSchedule(str(command2))


# sigController.stopCommandScheduler()


# result = sigController.getActiveTimingPlan()

# j = json.dumps(result, indent=4)

# f = open("CurrentTimingPlan.json", 'w')
# f.write(j)
