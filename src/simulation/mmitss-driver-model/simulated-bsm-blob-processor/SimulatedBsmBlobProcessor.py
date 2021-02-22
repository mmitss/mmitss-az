"""
**********************************************************************************

 © 2021 Arizona Board of Regents on behalf of the University of Arizona with rights
       granted for USDOT OSADP distribution with the Apache 2.0 open source license.

**********************************************************************************

  SimulatedBsmBlobProcessor.py  
  Created by: Niraj Vasant Altekar
  University of Arizona   
  College of Engineering

  This code was developed under the supervision of Professor Larry Head
  in the Systems and Industrial Engineering Department.

**********************************************************************************
"""
import bitstring
import json
import time, datetime

# Define constant multipliers:
MULTIPLIER_LATITUDE     =   10000000
MULTIPLIER_LONGITUDE    =   10000000
MULTIPLIER_ELEVATION    =   100
MULTIPLIER_SPEED        =   100
MULTIPLIER_HEADING      =   100
MULTIPLIER_LENGTH       =   10
MULTIPLIER_WIDTH        =   10

# Define dictioary of vehicle types:
VEH_TYPE_DICT           = {0:"unknown", 2: "EmergencyVehicle", 4: "Car", 6: "Transit", 9: "Truck"}

class SimulatedBsmBlobProcessor:
    """
    provides methods to: 
    (1) pack the BSM information (either from individual variables or JSON string) into a blob
    (2) unpack the received blob and develop BSM json string
    """
    def __init__(self):
        self.blobStructure = 'uint:32, uint:64, uint:16, int:32, int:32, int:32, uint:16, uint:16, uint:16, uint:16, uint:8'

    def pack_variables_to_blob(self, 
                                msgCount:int, 
                                temporaryId:int, 
                                secMark:int, 
                                latitude_DecimalDegree:float, 
                                longitude_DecimalDegree:float,
                                elevation_Meter:float, 
                                speed_MeterPerSecond:float, 
                                heading_Degree:float, 
                                length_cm:float, 
                                width_cm:float, 
                                vehicle_Type:(int or str)) -> bytes:
        """
        packs the information received in arguments into a blob and returns the blob
        """
        # Multiply with appropriate multipliers to convert floats into ints without loosing the information:
        latitude_DecimalDegree = latitude_DecimalDegree * MULTIPLIER_LATITUDE
        longitude_DecimalDegree = longitude_DecimalDegree * MULTIPLIER_LONGITUDE
        elevation_Meter = elevation_Meter * MULTIPLIER_ELEVATION
        speed_MeterPerSecond = speed_MeterPerSecond * MULTIPLIER_SPEED
        heading_Degree = heading_Degree *  MULTIPLIER_HEADING
        length_cm = length_cm * MULTIPLIER_LENGTH
        width_cm = width_cm * MULTIPLIER_WIDTH

        # Validate the vehicle type:
        if type(vehicle_Type) == str and vehicle_Type in VEH_TYPE_DICT.values():                
            vehicle_Type = dict(zip(VEH_TYPE_DICT.values(),VEH_TYPE_DICT.keys()))[vehicle_Type]
        elif type(vehicle_Type) == int and vehicle_Type in VEH_TYPE_DICT.keys():
            vehicle_Type = vehicle_Type
        else: vehicle_Type = 0

        # Pack the information into a blob:
        blob = bitstring.pack(self.blobStructure, msgCount, temporaryId, secMark, latitude_DecimalDegree, longitude_DecimalDegree,
                        elevation_Meter, speed_MeterPerSecond, heading_Degree, length_cm, width_cm, vehicle_Type).bytes
        
        # Return the blob:
        return blob
    
    def pack_json_to_blob(self, bsmJson:str) -> (bytes or bool):
        """
        packs the information in the BSM Json string received in the arguments into a blob and returns the blob.
        If JSON string having MsgType other than BSM, then returns False.
        """         
        # Load the received JSON string into a JSON object:
        bsmJson = json.loads(bsmJson)

        if bsmJson["MsgType"] == "BSM":
            msgCount = 0
            temporaryId = bsmJson["BasicVehicle"]["temporaryID"]
            secMark = bsmJson["BasicVehicle"]["secMark_Second"]
            latitude_DecimalDegree = bsmJson["BasicVehicle"]["position"]["latitude_DecimalDegree"] * MULTIPLIER_LATITUDE
            longitude_DecimalDegree = bsmJson["BasicVehicle"]["position"]["longitude_DecimalDegree"] * MULTIPLIER_LONGITUDE
            elevation_Meter = bsmJson["BasicVehicle"]["position"]["elevation_Meter"] * MULTIPLIER_ELEVATION
            speed_MeterPerSecond = bsmJson["BasicVehicle"]["speed_MeterPerSecond"] * MULTIPLIER_SPEED
            heading_Degree = bsmJson["BasicVehicle"]["heading_Degree"] * MULTIPLIER_HEADING
            length_cm = bsmJson["BasicVehicle"]["size"]["length_cm"] * MULTIPLIER_LENGTH
            width_cm = bsmJson["BasicVehicle"]["size"]["width_cm"] * MULTIPLIER_WIDTH
            vehicle_Type = bsmJson["BasicVehicle"]["type"]
            
            # Validate the vehicle type:
            if type(vehicle_Type) == str and vehicle_Type in VEH_TYPE_DICT.values():                
                vehicle_Type = dict(zip(VEH_TYPE_DICT.values(),VEH_TYPE_DICT.keys()))[vehicle_Type]
            elif type(vehicle_Type) == int and vehicle_Type in VEH_TYPE_DICT.keys():
                vehicle_Type = vehicle_Type
            else: vehicle_Type = 0

            # Pack the information into a blob:
            blob = bitstring.pack(self.blobStructure, msgCount, temporaryId, secMark, latitude_DecimalDegree, longitude_DecimalDegree,
                                    elevation_Meter, speed_MeterPerSecond, heading_Degree, length_cm, width_cm, vehicle_Type).bytes
            return blob
        else:
            return False
    
    def unpack_blob_to_json(self, blob:bytes) -> str:
        """
        unpacks the BSM_Blob received from VISSIM and returns the JSON string filled with received values
        """
        # Unpack the blob:
        blob = bitstring.BitStream(blob).unpack(self.blobStructure)

        # Validate the vehicle type and convert it into a string:
        if blob[10] in VEH_TYPE_DICT.keys():
            vehicle_Type = VEH_TYPE_DICT[blob[10]]
        else: vehicle_Type = VEH_TYPE_DICT[0]
        
        # Formulate JSON object containing BSM information:
        bsmJson = {
                    "MsgType": "BSM",
                    "Timestamp_posix": time.time(),
                    "Timestamp_verbose": str(datetime.datetime.now()),
                    "BasicVehicle":
                    {
                        "msgCount" : blob[0],
                        "temporaryID": blob[1],
                        "secMark_Second": blob[2],
                        "speed_MeterPerSecond": (blob[6] / MULTIPLIER_SPEED),
                        "heading_Degree": (blob[7] / MULTIPLIER_HEADING),
                        "type": vehicle_Type,
                        "position":
                        {
                            "latitude_DecimalDegree": (blob[3] / MULTIPLIER_LATITUDE), 
                            "longitude_DecimalDegree": (blob[4] / MULTIPLIER_LONGITUDE), 
                            "elevation_Meter": (blob[5] / MULTIPLIER_ELEVATION)
                        },
                        "size":
                        {
                            "length_cm": (blob[8] / MULTIPLIER_LENGTH),
                            "width_cm": (blob[9] / MULTIPLIER_WIDTH)
                        }
                    }                                    
                }
        # Convert the JSON object into a JSON string:
        bsmJson = json.dumps(bsmJson)

        # Return the JSON string:
        return bsmJson

if __name__ == "__main__":
    msgCount                =   122 # uint8
    temporaryId             =   32543223 # uint64
    secMark                 =   59600 # uint16
    latitude_DecimalDegree  =   33.8391521 # int32
    longitude_DecimalDegree =   -112.1361609 # int32
    elevation_Meter         =   507.02 # int32
    speed_MeterPerSecond    =   18.45 # uint16
    heading_Degree          =   38.56 # uint16
    length_cm               =   421.1 # uint16
    width_cm                =   200.3 # uint16
    vehicle_Type            =   9 # uint16 ONLY VALID VALUES: 0, 2, 4, 6, 9

    processor = SimulatedBsmBlobProcessor()

    # Create BLOB from plain variables:
    originalBlob = processor.pack_variables_to_blob(msgCount, temporaryId, secMark, latitude_DecimalDegree, longitude_DecimalDegree,
                                    elevation_Meter, speed_MeterPerSecond, heading_Degree, length_cm, width_cm, vehicle_Type)
    print(len(originalBlob))

    # Unpack BLOB into a BSM-JSON:
    bsmJson = processor.unpack_blob_to_json(originalBlob)
    print(bsmJson)

    # Pack BSM-JSON into a BLOB
    newBlob = processor.pack_json_to_blob(bsmJson)
    print(len(newBlob))



