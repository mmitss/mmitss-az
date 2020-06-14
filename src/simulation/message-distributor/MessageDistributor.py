import time, datetime
import json
import socket
import haversine

class MessageDistributor():
    def __init__(self, config:json):
        self.config = config
        self.intersectionList=config["intersections"]
        self.transit_client_list=self.getBsmAdditionalClientsList("transit")
        self.truck_client_list=self.getBsmAdditionalClientsList("truck")
        self.emergency_client_list=self.getBsmAdditionalClientsList("emergency")
        self.passenger_client_list=self.getBsmAdditionalClientsList("passenger")
        self.sendingSocket=socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

    def getBsmAdditionalClientsList(self, vehicleType:str):
        clients_list = []
        for client in self.config["bsm_additional_clients"][vehicleType]:
            client_tuple = ((client["ip_address"]), client["port"])
            clients_list = clients_list + [client_tuple]
        return clients_list

    def timestampMessage(self, message:json):
        message["timestamp_posix"] = time.time()
        message["timestamp_verobose"] = str(datetime.datetime.now())       
        return message
    
    def distributeBsmToClients(self, timestampedBsm:json):
        bsmVehicleType = timestampedBsm["type"]
        clientList=[]

        if bsmVehicleType == "transit":
            clientList = self.transit_client_list
        elif bsmVehicleType == "truck":
            clientList = self.truck_client_list
        elif bsmVehicleType == "emergency":
            clientList = self.emergency_client_list
        elif bsmVehicleType == "passenger":
            clientList = self.passenger_client_list
        
        if len(clientList)>0:
            for client in clientList:
                self.sendingSocket.sendto((json.dumps(timestampedBsm)).encode(), client)

   
    def distributeMsgToInfrastructureAndGetType(self, timestampedMessage:json):
        messageType = timestampedMessage["MsgType"]

        if messageType == "SRM":
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
                    self.sendingSocket.sendto((json.dumps(timestampedMessage)).encode(), (intersection["ip_address"], intersection["srm_client_port"]))
                elif messageType == "BSM": 
                    self.sendingSocket.sendto((json.dumps(timestampedMessage)).encode(), (intersection["ip_address"], intersection["bsm_client_port"]))
                
        return messageType
  
    def __del__(self):
        self.sendingSocket.close()
    