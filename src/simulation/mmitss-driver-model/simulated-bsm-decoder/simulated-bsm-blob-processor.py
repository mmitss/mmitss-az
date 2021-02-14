import socket
import time, datetime
from SimulatedBsmBlobProcessor import SimulatedBsmBlobProcessor

processor = SimulatedBsmBlobProcessor()

fp = open("SimulatedBsmLog.csv", 'w')
fp.write("log_timestamp_verbose,log_timestamp_posix,timestamp_verbose,timestamp_posix,"
                                    + "temporaryId,secMark,latitude,longitude,elevation,speed,heading,type,length,width\n")
s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
s.bind(("10.12.6.251"),5060)

while True:
    data, address = s.recvfrom(128)
    bsmJson = processor.unpack_blob_to_json(data)
    log_timestamp_verbose = str(datetime.datetime.now())
    log_timestamp_posix = str(time.time())
    timestamp_verbose = str(bsmJson["Timestamp_verbose"])
    timestamp_posix = str(bsmJson["Timestamp_posix"])
    temporaryId = str(bsmJson["BasicVehicle"]["temporaryID"])
    secMark = str(bsmJson["BasicVehicle"]["secMark_Second"])
    latitude = str(bsmJson["BasicVehicle"]["position"]["latitude_DecimalDegree"])
    longitude = str(bsmJson["BasicVehicle"]["position"]["longitude_DecimalDegree"])
    elevation = str(bsmJson["BasicVehicle"]["position"]["elevation_Meter"])
    speed = str(bsmJson["BasicVehicle"]["speed_MeterPerSecond"])
    heading = str(bsmJson["BasicVehicle"]["heading_Degree"])
    vehType = str(bsmJson["BasicVehicle"]["type"])
    length = str(bsmJson["BasicVehicle"]["size"]["length_cm"])
    width = str(bsmJson["BasicVehicle"]["size"]["width_cm"])
    csv = (log_timestamp_verbose + "," 
                + log_timestamp_posix + "," 
                + timestamp_verbose + "," 
                + timestamp_posix + "," 
                + temporaryId + "," 
                + secMark + "," 
                + latitude + "," 
                + longitude + "," 
                + elevation + "," 
                + speed + "," 
                + heading + "," 
                + vehType + ","
                + length + "," 
                + width + "\n")
    fp.write(csv)
fp.close()
socket.close()

