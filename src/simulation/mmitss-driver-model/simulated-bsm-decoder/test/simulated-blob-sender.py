import bitstring
import socket

MULTIPLIER_LATITUDE     =   10000000
MULTIPLIER_LONGITUDE    =   10000000
MULTIPLIER_ELEVATION    =   100
MULTIPLIER_SPEED        =   100
MULTIPLIER_HEADING      =   100
MULTIPLIER_LENGTH       =   10
MULTIPLIER_WIDTH        =   10

def pack_blob(msgCount_in, temporaryId_in, secMark_in, latitude_DecimalDegree_in,
                longitude_DecimalDegree_in, elevation_Meter_in, speed_MeterPerSecond_in,
                heading_Degree_in, length_cm_in, width_cm_in, vehicle_Type_in):           
    blob = bitstring.pack('uint:8, uint:64, uint:16, int:32, int:32, int:32, uint:16, uint:16, uint:16, uint:16, uint:8',
                            msgCount_in, temporaryId_in, secMark_in,
                            (latitude_DecimalDegree_in * MULTIPLIER_LATITUDE),
                            (longitude_DecimalDegree_in * MULTIPLIER_LONGITUDE),
                            (elevation_Meter_in * MULTIPLIER_ELEVATION),
                            (speed_MeterPerSecond_in * MULTIPLIER_SPEED),
                            (heading_Degree_in * MULTIPLIER_HEADING),
                            (length_cm_in * MULTIPLIER_LENGTH),
                            (width_cm_in * MULTIPLIER_WIDTH),
                            vehicle_Type_in).bytes
    return blob

if __name__ == "__main__":
    client = (("10.12.6.252", 5001))
    msgCount_in                =   122 # uint8
    temporaryId_in             =   32543223 # uint64
    secMark_in                 =   59600 # uint16
    latitude_DecimalDegree_in  =   33.8391521 # int32
    longitude_DecimalDegree_in =   -112.1361609 # int32
    elevation_Meter_in         =   507.02 # int32
    speed_MeterPerSecond_in    =   18.45 # uint16
    heading_Degree_in          =   38.56 # uint16
    length_cm_in               =   421.1 # uint16
    width_cm_in                =   200.3 # uint16
    vehicle_Type_in            =   2 # uint16

    blob = pack_blob(msgCount_in, temporaryId_in, secMark_in, latitude_DecimalDegree_in,
                longitude_DecimalDegree_in, elevation_Meter_in, speed_MeterPerSecond_in,
                heading_Degree_in, length_cm_in, width_cm_in, vehicle_Type_in)

    s =socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    s.sendto(blob, client)
