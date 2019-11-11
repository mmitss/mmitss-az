'''
**********************************************************************************

 © 2019 Arizona Board of Regents on behalf of the University of Arizona with rights
       granted for USDOT OSADP distribution with the Apache 2.0 open source license.

**********************************************************************************

  MsgSender.py  
  Created by: Niraj Vasant Altekar
  University of Arizona   
  College of Engineering

  This code was developed under the supervision of Professor Larry Head
  in the Systems and Industrial Engineering Department.

  Revision History:
  1. This is an initial revision. MsgSender application performs following tasks:
    -> Reads configuration items from the config file: Self IPaddress and IP address of sourceDsrcDevice
    -> Listens for message payloads at UDP socket
    -> Once a message is received, identifies its type
    -> Based on the identified type, formulates entire message packet.
    -> Sends the developed message packet to sourceDsrcDevice.
    -> Creates a blank message packet if an unknown message is received.
'''

import socket
import MsgSenderConfig

def identifyMsg(uperString:str):
    if uperString[:4] == '0014':
        msgType = 'BSM'
    elif uperString[:4] == '0013':
        msgType = 'SPAT'
    elif (uperString[:4] == '001D' or uperString[:4] == '001d'):
        msgType = 'SRM'
    elif (uperString[:4] == '001E' or uperString[:4] == '001e'):
        msgType = 'SSM'
    elif uperString[:4] == '0012':
        msgType = 'MAP'
    elif (uperString[:4] == '001F' or uperString[:4] == '001f'):
        msgType = 'TIM'
    elif uperString[:4] == '0021':
        msgType = 'RSM'
    else: msgType = 'unknown'
    return msgType

def createBroadcastMsgPacket(msgType, msgPayload):
    broadcastMsgPacket = '''Version={}
Type={}
PSID={}
Priority={}
TxMode={}
TxChannel={}
TxInterval={}
DeliveryStart={}
DeliveryStop={}
Signature={}
Encryption={}
Payload={}'''

    if msgType == 'MAP':
        broadcastMsgPacket = broadcastMsgPacket.format(0.7, msgType, '0xE0000017', 7, 'CONT', 172, '', '', '', False, False, msgPayload)
    elif msgType == 'SPAT':
        broadcastMsgPacket = broadcastMsgPacket.format(0.7, msgType, '0x8002', 7, 'CONT', 172, '', '', '', False, False, msgPayload)
    elif msgType == 'SSM':
        broadcastMsgPacket = broadcastMsgPacket.format(0.7, msgType, '0xE0000020', 7, 'ALT', 182, '', '', '', False, False, msgPayload)
    elif msgType == 'SRM':
        broadcastMsgPacket = broadcastMsgPacket.format(0.7, msgType, '0xE0000019', 7, 'ALT', 182, '', '', '', False, False, msgPayload)
    elif msgType == 'BSM':
        broadcastMsgPacket = broadcastMsgPacket.format(0.7, msgType, '0x20', 7, 'CONT', 172, '', '', '', False, False, msgPayload)
    elif msgType == 'RSM':
        broadcastMsgPacket = broadcastMsgPacket.format(0.7, msgType, '0x8003', 7, 'CONT', 178, '', '', '', False, False, msgPayload)
    elif msgType == 'TIM':
        broadcastMsgPacket = broadcastMsgPacket.format(0.7, msgType, '0x8003', 7, 'CONT', 178, '', '', '', False, False, msgPayload)
    else:broadcastMsgPacket=''
                        
    return broadcastMsgPacket


def main():
    
    config = MsgSenderConfig.MsgSenderConfig("MsgSenderConfig.json")
    
    msgSenderIP = config.getCoProcessorIP()
    msgSenderPort = 10001
    msgSender = (msgSenderIP, msgSenderPort)
    # Create a socket
    s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    # Bind the created socket to the server information.
    s.bind(msgSender)

    sourceDsrcDeviceIP = config.getSourceDsrcDeviceIP()
    sourceDsrcDevicePort = 1516
    sourceDsrcDevice = (sourceDsrcDeviceIP, sourceDsrcDevicePort)

    while True:
        receivedMsg, addr = s.recvfrom(2048)
        print(receivedMsg.decode())
        msgType = identifyMsg(receivedMsg.decode())
        msgPacket = createBroadcastMsgPacket(msgType, receivedMsg.decode())
        s.sendto(msgPacket.encode(), sourceDsrcDevice)
        print("Sent " + msgType + msgPacket)

if __name__ == '__main__':
    main()

