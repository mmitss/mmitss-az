import sys
import os
import json
import haversine
import pandas as pd
from Position3D import Position3D
from geoCoord import GeoCoord

class LocalCoordinatesProcessor:
    def __init__(self, inputFile:str):
        self.inputFile = inputFile
        self.bsmDf = pd.read_csv(inputFile)
        self.referenceLocation = self.get_reference_location()

    def get_reference_location(self):
        with open("./../config/map-bsm-processor-config.json", 'r') as configFile:
            config = json.load(configFile)

        latitude = config["ReferenceLocation"]["Latitude_DecimalDegree"]
        longitude = config["ReferenceLocation"]["Longitude_DecimalDegree"]
        elevation = config["ReferenceLocation"]["Elevation_Meter"]

        return Position3D(latitude, longitude, elevation)

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
            tempDf['latitude_prev'] = tempDf['latitude'].shift(1)
            tempDf['longitude_prev'] = tempDf['longitude'].shift(1)
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
        #1
        """
        This function processed trajectory signal groups for all data points. 
        """
        bsmDfs = []
        self.bsmDf["trajectory_signal_group"] = 0

        for vehId in self.bsmDf.temporaryId.unique():
            tempDf = self.bsmDf.loc[self.bsmDf["temporaryId"]==vehId]
            trajectorySignalGroup = 0
            applicableSignalGroups = (tempDf["current_signal_group"].unique())
            applicableSignalGroups.sort()
            
            if min(applicableSignalGroups)==0:
                minTrajectorySignalGroup = applicableSignalGroups[1]
            else: minTrajectorySignalGroup = min(applicableSignalGroups)
            
            maxTrajectorySignalGroup = max(tempDf["current_signal_group"].unique())
            
            if minTrajectorySignalGroup == maxTrajectorySignalGroup:
                trajectorySignalGroup = minTrajectorySignalGroup
            elif minTrajectorySignalGroup == 3 and maxTrajectorySignalGroup == 8:
                trajectorySignalGroup = 3                
            elif minTrajectorySignalGroup == 1 and maxTrajectorySignalGroup == 6:
                trajectorySignalGroup = 1                
            elif minTrajectorySignalGroup == 4 and maxTrajectorySignalGroup == 7:
                trajectorySignalGroup = 7                
            elif minTrajectorySignalGroup == 2 and maxTrajectorySignalGroup == 5:
                trajectorySignalGroup = 5
                            
            tempDf["trajectory_signal_group"] = trajectorySignalGroup
            bsmDfs = bsmDfs + [tempDf]
            
        self.bsmDf = pd.concat(bsmDfs)


if __name__ == "__main__":

    if len(sys.argv) == 1:
        print("Missing argument: input file!")
        exit()
    elif len(sys.argv) > 2:
        print("Too many arguments. Only required argument is the location of the input file!")
        exit()
    elif not os.path.exists(sys.argv[1]):
        print("Input file does not exist!")
        exit()
    inputFile = sys.argv[1]
    lcp = LocalCoordinatesProcessor(inputFile)
    
    lcp.process_local_coordinates()
    lcp.process_distance_along_path()
    lcp.process_in_queue_status()
    lcp.process_time_to_stopbar()
    lcp.process_trajectory_signal_groups()

    lcp.bsmDf.to_csv(inputFile,index=False)
    
