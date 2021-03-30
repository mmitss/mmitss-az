import sys
import os
import json
import haversine
import pandas as pd
from Position3D import Position3D
from geoCoord import GeoCoord

class LocalCoordinatesProcessor:
    def __init__(self, inputFile:str, configFile:str):
        self.inputFile = inputFile
        self.configFile = configFile
        self.bsmDf = pd.read_csv(inputFile)
        self.referenceLocation = self.get_reference_location()

    def get_reference_location(self):
        with open(self.configFile, 'r') as configFile:
            config = json.load(configFile)

        latitude = config["ReferenceLocation"]["Latitude_DecimalDegree"]
        longitude = config["ReferenceLocation"]["Longitude_DecimalDegree"]
        elevation = config["ReferenceLocation"]["Elevation_Meter"]

        return Position3D(latitude, longitude, elevation)

    def process_onmap_status(self):
        self.bsmDf["onmap_status"] = True

    def process_in_queue_status(self):
        def in_queue_status(row, speed_threshold):
            if (row["speed"] < speed_threshold and row["position_on_map"] == "inbound"):
                return True
            else: return False

        speed_threshold = 2
        self.bsmDf["in_queue"] = self.bsmDf.apply(lambda row: in_queue_status(row, speed_threshold), axis=1)
        
    def process_time_to_stopbar(self):
        def time_to_stopbar(row):
            if row["in_queue"]==False and row["position_on_map"] == "inbound":
                return row["dist_to_stopbar"]/row["speed"]
            else: return False       
        self.bsmDf["time_to_stopbar"] = self.bsmDf.apply(lambda row: time_to_stopbar(row), axis=1)
        
    def process_distance_along_path(self):
        def distance_from_prev_point(row):
            distance = haversine.haversine((row["latitude_prev"], row["longitude_prev"]),(row["latitude"], row["longitude"]),unit=haversine.Unit.METERS)
            return distance        

        bsmDfs = []        
        for vehId in self.bsmDf.temporaryId.unique():
            tempDf = self.bsmDf.loc[self.bsmDf["temporaryId"]==vehId]
            tempDf['latitude_prev'] = tempDf['latitude'].shift()
            tempDf['longitude_prev'] = tempDf['longitude'].shift()
            tempDf = tempDf[pd.notnull(tempDf["latitude_prev"])]
            tempDf["dist_from_prev_pt"] = tempDf.apply(lambda row: distance_from_prev_point(row), axis=1)
            tempDf["distance_along_path"] = tempDf["dist_from_prev_pt"].cumsum()
            tempDf = tempDf.drop(columns=['latitude_prev', 'longitude_prev', 'dist_from_prev_pt'])
            bsmDfs = bsmDfs + [tempDf]
        self.bsmDf = pd.concat(bsmDfs)

    def process_local_coordinates(self):

        geo = GeoCoord(self.referenceLocation)

        def get_localX(row):
            ex, ey, ez = geo.lla2ecef(Position3D(row["latitude"], row["longitude"], row["elevation"]))
            x, y, z = geo.ecef2local(ex, ey, ez)
            return x

        def get_localY(row):
            ex, ey, ez = geo.lla2ecef(Position3D(row["latitude"], row["longitude"], row["elevation"]))
            x, y, z = geo.ecef2local(ex, ey, ez)
            return y

        def get_localZ(row):
            ex, ey, ez = geo.lla2ecef(Position3D(row["latitude"], row["longitude"], row["elevation"]))
            x, y, z = geo.ecef2local(ex, ey, ez)
            return z

        self.bsmDf["local_x"] = self.bsmDf.apply(lambda row: get_localX(row), axis=1)
        self.bsmDf["local_y"] = self.bsmDf.apply(lambda row: get_localY(row), axis=1)
        self.bsmDf["local_z"] = self.bsmDf.apply(lambda row: get_localZ(row), axis=1)   


    def process_trajectory_signal_groups(self):
        def get_trajectory_signal_group_for_row(row, lastInboundPhase):
            if row["position_on_map"]=="inbound":
                return row["current_signal_group"]
            else: 
                return lastInboundPhase
        
        def fill_trajectory_signal_group_for_vehicle(vehicleDf):
            tempDf = vehicleDf.loc[vehicleDf["position_on_map"]=="inbound"]
    
            lastInboundPhase = int(tempDf.loc[tempDf["timestamp_posix"]==max(tempDf["timestamp_posix"])]["current_signal_group"])
            vehicleDf["trajectory_signal_group"] = vehicleDf.apply(lambda row: get_trajectory_signal_group_for_row(row, lastInboundPhase), axis=1)
            return vehicleDf
        
        self.bsmDf = self.bsmDf.groupby("temporaryId").apply(fill_trajectory_signal_group_for_vehicle)
        
    def process_distances_to_stopbar_for_inside_box(self):
        
        def get_dist_to_stopbar_for_row_inside_box(row, lastInboundPoint):
            originalDistToStopbar = float(row["dist_to_stopbar"])
            if originalDistToStopbar == 0:
                currentPoint = (row["latitude"], row["longitude"])
                distToStopbar = (haversine.haversine(lastInboundPoint, currentPoint, haversine.Unit.METERS))*(-1)
                return distToStopbar
            else: return originalDistToStopbar
        
        def fill_dist_to_stopbar_for_vehicle(vehicleDf):
            tempDf = vehicleDf.loc[vehicleDf["position_on_map"]=="inbound"]
            lastInboundLatitude = float(tempDf.loc[tempDf["timestamp_posix"]==max(tempDf["timestamp_posix"])]["latitude"])
            lastInboundLongitude = float(tempDf.loc[tempDf["timestamp_posix"]==max(tempDf["timestamp_posix"])]["longitude"])
            lastInboundPoint = (lastInboundLatitude, lastInboundLongitude)
            vehicleDf["dist_to_stopbar"] = vehicleDf.apply(lambda row: get_dist_to_stopbar_for_row_inside_box(row, lastInboundPoint), axis=1)            
            return vehicleDf
        
        self.bsmDf = self.bsmDf.groupby("temporaryId").apply(fill_dist_to_stopbar_for_vehicle)
        
if __name__ == "__main__":
    
    DEBUGGING = False
    

    if not DEBUGGING:
        if len(sys.argv) == 1:
            print("Two arguments are required: input file and config file!")
            exit()
        elif len(sys.argv) > 3:
            print("Too many arguments. Only required arguments are the locations of the input file and config file")
            exit()
        elif not os.path.exists(sys.argv[1]):
            print("Input file does not exist!")
            exit()
        elif not os.path.exists(sys.argv[2]):
            print("Config file does not exist!")
            exit()
    
        inputFile = sys.argv[1]
        configFile = sys.argv[2]
    else: 
        inputFile = "/home/nvaltekar/repo/safety-assessment/data/simulation/base/processedRemoteBsmLog.csv"
        configFile = "./../config/daisy-gavilan.json"
    
    lcp = LocalCoordinatesProcessor(inputFile, configFile)
    
    
    lcp.process_onmap_status()
    lcp.process_local_coordinates()
    lcp.process_distance_along_path()
    lcp.process_in_queue_status()
    lcp.process_time_to_stopbar()
    lcp.process_trajectory_signal_groups()
    lcp.process_distances_to_stopbar_for_inside_box()

    lcp.bsmDf.to_csv(inputFile,index=False)
    
