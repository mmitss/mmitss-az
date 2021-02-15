'''
**********************************************************************************

 © 2019 Arizona Board of Regents on behalf of the University of Arizona with rights
       granted for USDOT OSADP distribution with the Apache 2.0 open source license.

**********************************************************************************

  MessageDistributor.py  
  Created by: Niraj Vasant Altekar
  University of Arizona   
  College of Engineering

  This code was developed under the supervision of Professor Larry Head
  in the Systems and Industrial Engineering Department.

**********************************************************************************
'''
import time, datetime
import json
import socket
import haversine
import random

class MessageDistributor():
    """
    provides methods for distribution of BSMs and SRMs to multiple clients. 
      
    ``config`` is the json object of the configuration file.
    """
    def __init__(self, config:json):
        self.config = config
        self.intersectionList=config["intersections"]

        # BSM Clients:
        self.transit_client_list=self.getBsmClientsList("Transit")
        self.truck_client_list=self.getBsmClientsList("Truck")
        self.emergency_client_list=self.getBsmClientsList("EmergencyVehicle")
        self.passenger_client_list=self.getBsmClientsList("Passenger")
        self.other_client_list=self.getBsmClientsList("other")

        # MAP Clients:
        self.map_client_list=self.getMapClientsList()

        # SSM Clients:
        self.ssm_client_list=self.getSsmClientsList()
        
        ''' This socket is used only for outgoing messages. The socket for incoming 
            messages needs to be opened (and closed) in the wrapper module
        ''' 
        self.sendingSocket=socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

        # 
        if config["package_drop_probability"] > 1 or config["package_drop_probability"] < 0:
            print("Invalid package drop probability. Use a number between 0 and 1. Using default (=0) for this session!")
            self.packageDropProbability = 0
        else: self.packageDropProbability = config["package_drop_probability"]
    def getBsmClientsList(self, vehicleType:str):
        """
        reads the information of clients for the vehicle type specified in the argument,
        and returns the list of tuples having client information in form of 
        (IpAddress:str, Port:int) 

        ``vehicleType`` can be one of "Transit", "Truck", "EmergencyVehicle", "Passenger" or something else.
        """
        clients_list = []
        for client in self.config["bsm_clients"][vehicleType]:
            client_tuple = ((client["ip_address"]), client["port"])
            clients_list = clients_list + [client_tuple]
        return clients_list

    def getSsmClientsList(self):

        clients_list = []
        for client in self.config["ssm_clients"]:
            client_tuple = ((client["ip_address"]), client["port"])
            clients_list = clients_list + [client_tuple]
        return clients_list

    def getMapClientsList(self):

        clients_list = []
        for client in self.config["map_clients"]:
            client_tuple = ((client["ip_address"]), client["port"])
            clients_list = clients_list + [client_tuple]
        return clients_list

    def timestampMessage(self, message:json):
        """
        adds (or updates) the unix-epoch and verbose timestamps to the received message, 
        and returns the type of the messaye from "MsgType" key.
        The unix-epoch timestamp is added as a value to key "timestamp_posix", whereas
        the verbose timestamp (yyyy-mm-dd hh:mm:ss.ss) is added as a value to key
        "timestamp_verbose".

        ``message`` is a JSON object of the message
        """
        message["Timestamp_posix"] = time.time()
        message["Timestamp_verbose"] = str(datetime.datetime.now())       
        return message
    
    def distributeBsmToClients(self, timestampedBsm:json):
        """
        distributes the BSM provided in the arguments to clients based on 
        the type of the vehicle from which the BSM was received.

        ``timestampedBsm`` is a JSON object of the timestamped BSM
        """
        bsmVehicleType = timestampedBsm["BasicVehicle"]["type"]
        clientList=[]

        if bsmVehicleType == "Transit":
            clientList = self.transit_client_list
        elif bsmVehicleType == "Truck":
            clientList = self.truck_client_list
        elif bsmVehicleType == "EmergencyVehicle":
            clientList = self.emergency_client_list
        elif bsmVehicleType == "Passenger":
            clientList = self.passenger_client_list
        else:clientList = self.other_client_list
        
        if len(clientList)>0:
            for client in clientList:
                self.send_message_to_client((json.dumps(timestampedBsm)).encode(), client)

    def distributeMapToClients(self, timestampedMsg:json):
        clientList = self.map_client_list
        if len(clientList)>0:
            for client in clientList:
                self.send_message_to_client((json.dumps(timestampedMsg)).encode(), client)

    def distributeSsmToClients(self, timestampedMsg:json):
        clientList = self.ssm_client_list
        if len(clientList)>0:
            for client in clientList:
                self.send_message_to_client((json.dumps(timestampedMsg)).encode(), client)

   
    def distributeMsgToInfrastructureAndGetType(self, timestampedMessage:json):
        """
        distributes the message to infrastructural clients (intersections) and 
        returns the type of the message. The message is distributed to an intersection
        only if the haversine distance between that intersection and the vehicle's 
        current location is less than the DSRC range of the intersection.

        ``timestampedMessage`` is a JSON object of the timestamped message
        """
        messageType = timestampedMessage["MsgType"]

        if((messageType == "SSM") or (messageType == "MAP")):
            return messageType

        elif messageType == "SRM":
            print("Received SRM")
            position = timestampedMessage["SignalRequest"]["position"]
        elif messageType == "BSM":
            position = timestampedMessage["BasicVehicle"]["position"]

        msgLatitude = position["latitude_DecimalDegree"]
        msgLongitude = position["longitude_DecimalDegree"]

        # Distribute the timestampedMessage to nearby intersections:
        for intersection in self.intersectionList:
            
            intersectionLatitude = intersection["position"]["latitude_DecimalDegree"]
            intersectionLongitude = intersection["position"]["longitude_DecimalDegree"]

            msgDistance = haversine.haversine((msgLatitude, msgLongitude), 
                                                (intersectionLatitude, intersectionLongitude), 
                                                unit=haversine.Unit.METERS)

            if msgDistance <= intersection["dsrc_range_Meter"]:
                if messageType == "SRM":
                    self.send_message_to_client((json.dumps(timestampedMessage)).encode(), (intersection["ip_address"], intersection["srm_client_port"]))
                    print("Distributed SRM")
                elif messageType == "BSM": 
                    self.send_message_to_client((json.dumps(timestampedMessage)).encode(), (intersection["ip_address"], intersection["bsm_client_port"]))
                
        return messageType
    
    def send_message_to_client(self, message:bytes, client:tuple):
        rand = random.uniform(0,1)
        if rand < self.packageDropProbability:
            pass
        else:
            self.sendingSocket.sendto(message, client)
  
    def __del__(self):
        """
        closes the open sockets before destructing the object.
        """
        self.sendingSocket.close()
    