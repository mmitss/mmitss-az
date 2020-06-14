import time, datetime
import json
import socket
import haversine

print("Starting msg-distributor...")

# Open the config file and load the JSON string into a JSON object:
configFile = open("message_distributor_config.json", 'r')
config = json.load(configFile)
print("Successfully loaded the \"mmitss_simulation_config.json\" file")

# Create and bind an UDP socket for the M_BsmDistributor.py. Driver models will send the BSMs to this socket:
s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
msgDistributor_commInfo = (config["msg_distributor_ip"], config["msg_distributor_port"])
s.bind(msgDistributor_commInfo)
print("Successfully established a UDP socket listening for simulated msgs at: " + str(msgDistributor_commInfo))

# Extract the list of intersections:
intersectionList = config["intersections"]
print("Successfully parsed the list of intersections." +
            " This corridor has " + str(len(intersectionList)) + " intersection(s)")

# Extract the additional clients for each vehicle type:
# 1) For transit:
transit_clients_list = []
print("BSMs from transit vehicles will be sent to following additional clients:")
for client in config["bsm_additional_clients"]["transit"]:
    client_tuple = ((client["ip_address"]), client["port"])
    transit_clients_list = transit_clients_list + [client_tuple]
    print(client_tuple)
if len(transit_clients_list)==0: print("NONE")

# 2) For truck:
truck_clients_list = []
print("BSMs from truck vehicles will be sent to following additional clients:")
for client in config["bsm_additional_clients"]["truck"]:
    client_tuple = ((client["ip_address"]), client["port"])
    truck_clients_list = truck_clients_list + [client_tuple]
    print(client_tuple)
if len(truck_clients_list)==0: print("NONE")

# 3) For emergency:
emergency_clients_list = []
print("BSMs from emergency vehicles will be sent to following additional clients:")
for client in config["bsm_additional_clients"]["emergency"]:
    client_tuple = ((client["ip_address"]), client["port"])
    emergency_clients_list = emergency_clients_list + [client_tuple]
    print(client_tuple)
if len(emergency_clients_list)==0: print("NONE")

# 4) For passenger:
passenger_clients_list = []
print("BSMs from passenger vehicles will be sent to following additional clients:")
for client in config["bsm_additional_clients"]["passenger"]:
    client_tuple = ((client["ip_address"]), client["port"])
    passenger_clients_list = passenger_clients_list + [client_tuple]
    print(client_tuple)
if len(passenger_clients_list)==0: print("NONE")


while True:
    # Receive BSM from the running simulation and load it in a JSON object:
    data, addr = s.recvfrom(1024)
    bsm = json.loads(data.decode())
    
    # Extract the required info from the BSM:
    bsmLatitude = bsm["BasicVehicle"]["position"]["latitude_DecimalDegree"]
    bsmLongitude = bsm["BasicVehicle"]["position"]["longitude_DecimalDegree"]
    bsmVehicleType = bsm["BasicVehicle"]["type"]

    # Add the timestamp to the BSM and prepare the BSM string for sending to clients:
    bsm["timestamp_posix"] = time.time()
    bsm["timestamp_verobose"] = str(datetime.datetime.now())
    timestampedBsm = json.dumps(bsm)

    # Distribute the BSM to additional clients based on the type of the vehicle from which this BSM is received
    if bsmVehicleType == "transit":
        if len(transit_clients_list) > 0:
            for client in transit_clients_list:
                s.sendto(timestampedBsm.encode(), client)

    elif bsmVehicleType == "truck":
        if len(truck_clients_list) > 0:
            for client in truck_clients_list:
                s.sendto(timestampedBsm.encode(), client)
    
    elif bsmVehicleType == "emergency":
        if len(emergency_clients_list) > 0:
            for client in emergency_clients_list:
                s.sendto(timestampedBsm.encode(), client)
    
    elif bsmVehicleType == "passenger":
        if len(passenger_clients_list) > 0:
            for client in passenger_clients_list:
                s.sendto(timestampedBsm.encode(), client)

    # Distribute the BSM to nearby intersections:
    for intersection in intersectionList:
        
        intersectionLatitude = intersection["position"]["latitude_DecimalDegree"]
        intersectionLongitude = intersection["position"]["longitude_DecimalDegree"]

        bsmDistance = haversine.haversine((bsmLatitude, bsmLongitude), 
                                          (intersectionLatitude, intersectionLongitude), 
                                          unit=haversine.Unit.METERS)

        if bsmDistance <= intersection["dsrc_range_Meter"]:
            s.sendto(data, (intersection["ip_address"], intersection["bsm_client_port"]))
            print( "Sending BSM from vehicle " + str(bsm["BasicVehicle"]["temporaryID"]) + " to " + 
                                          intersection["name"] + ":" + str(int(bsmDistance))  + " meters away")
s.close()