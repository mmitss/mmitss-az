import bitstring
import json
import time, datetime

MULTIPLIER_LATITUDE     =   10000000
MULTIPLIER_LONGITUDE    =   10000000
MULTIPLIER_ELEVATION    =   100
MULTIPLIER_SPEED        =   100
MULTIPLIER_HEADING      =   100
MULTIPLIER_LENGTH       =   10
MULTIPLIER_WIDTH        =   10

class SimulatedBsmBlobProcessor:
    def __init__(self):
        self.blobStructure = 'uint:8, uint:64, uint:16, int:32, int:32, int:32, uint:16, uint:16, uint:16, uint:16, uint:8'

    def pack_variables_to_blob(self, msgCount, temporaryId, secMark, latitude_DecimalDegree, longitude_DecimalDegree,
                    elevation_Meter, speed_MeterPerSecond, heading_Degree, length_cm, width_cm, vehicle_Type):

        latitude_DecimalDegree = latitude_DecimalDegree * MULTIPLIER_LATITUDE
        longitude_DecimalDegree = longitude_DecimalDegree * MULTIPLIER_LONGITUDE
        elevation_Meter = elevation_Meter * MULTIPLIER_ELEVATION
        speed_MeterPerSecond = speed_MeterPerSecond * MULTIPLIER_SPEED
        heading_Degree = heading_Degree *  MULTIPLIER_HEADING
        length_cm = length_cm * MULTIPLIER_LENGTH
        width_cm = width_cm * MULTIPLIER_WIDTH
        
        blob = bitstring.pack(self.blobStructure, msgCount, temporaryId, secMark, latitude_DecimalDegree, longitude_DecimalDegree,
                        elevation_Meter, speed_MeterPerSecond, heading_Degree, length_cm, width_cm, vehicle_Type).bytes
        return blob
    
    def pack_json_to_blob(self, bsmJson:str):           
        
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
            blob = bitstring.pack(self.blobStructure, msgCount, temporaryId, secMark, latitude_DecimalDegree, longitude_DecimalDegree,
                                    elevation_Meter, speed_MeterPerSecond, heading_Degree, length_cm, width_cm, vehicle_Type).bytes
            return blob
        else:
            return False
    
    def unpack_blob_to_json(self, blob:bytes):

        blob = bitstring.BitStream(blob).unpack(self.blobStructure)

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
                        "type": blob[10],
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
        bsmJson = json.dumps(bsmJson)
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
    vehicle_Type            =   2 # uint16

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



