"""
**********************************************************************************

 © 2021 Arizona Board of Regents on behalf of the University of Arizona with rights
       granted for USDOT OSADP distribution with the Apache 2.0 open source license.

**********************************************************************************

  PullFromIntersection.py  
  Created by: Niraj Vasant Altekar
  University of Arizona   
  College of Engineering

  This code was developed under the supervision of Professor Larry Head
  in the Systems and Industrial Engineering Department.

**********************************************************************************
"""
# Import system libraries:
import os
import datetime
import json

# Import 3rd party libraries:
import pysftp

# Import local modules
from V2XDataTransfer import V2XDataTransfer
from Logger import Logger

class PullFromIntersection(V2XDataTransfer):
    """
    provides method to pull the dat a from multiple intersections and store it in local directories.
    This class is intended to be deployed on the server side.
    """
    def __init__(self, serverDataDirectory:str, intersectionList:str, logger:Logger):
        
        # Initialize the parent class
        super().__init__(serverDataDirectory, intersectionList, logger)
        
        # Verify and create the expected directory structure on the local machine
        self.verify_or_create_local_directory_structure()

    def verify_or_create_local_directory_structure(self):
        """
        verifies if the correct directory structure is available locally to store files pertaining to all 
        data elements from all intersections. The directory structure would look like the following:
        - RootDataDirectory
            - Intersection-1
                - msgCount
                - remoteBsm
                - spat
                - srm
                - ssm
        """

        # Check if directory structure exists for listed intersections. If not, create the required folders:
        for intersection in self.intersectionList:
            intersectionDirectory = self.serverDataDirectory + "/" + intersection["name"]
            for dataElement in self.dataElements:
                dataElementDirectory = intersectionDirectory + "/" + dataElement 
                if not os.path.isdir(dataElementDirectory):
                    os.makedirs(dataElementDirectory)
                    self.logger.write("Created local path " + dataElementDirectory)

    def transfer_data(self):
        """
        transfers the data from multiple intersections and stores them locally (pull from intersections)
        """
        # For each intersection (remote machine):
        for intersection in self.intersectionList:
            try:
                # Establish an SFTP connection
                with pysftp.Connection(intersection["ip_address"], username=intersection["username"], password=intersection["password"]) as sftp:
                    self.logger.write("Logged in to " + intersection["name"])
                    try:
                        
                        # On remote machine, change the working directory to v2x-data/archive
                        with sftp.cd(intersection["v2x-data_location"] + "/archive/"):
                            # List archived directories
                            remoteArchivedDirectories = sftp.listdir()
                            self.logger.write("On " + intersection["name"] + " found these directories: " + str(remoteArchivedDirectories))
                            
                            # For each archived directory:
                            for directory in remoteArchivedDirectories:
                               
                                # On remote machine, change the working directory to v2x-data/archive/archivedDirectory
                                with sftp.cd(intersection["v2x-data_location"] + "/archive/" + directory):
                                    
                                    # List all files available in the directory
                                    dataFiles = sftp.listdir()
                                    
                                    # Identify the data element associated with each available file, and store it in a "dataElementFiles" dictionary
                                    for dataElement in self.dataElements:
                                        filename = [file for file in dataFiles if dataElement in file]
                                        if len(filename) > 0:
                                            self.dataElementFiles[dataElement] = filename[0]
                                        else: self.dataElementFiles[dataElement] = None
                                    
                                    # For each data element:
                                    for dataElement in self.dataElements:
                                        # If the file exists:
                                        if not self.dataElementFiles[dataElement] == None:
                                            
                                            # Set the local path where file needs to be transferred:
                                            localpath=self.serverDataDirectory + "/" + intersection["name"] + "/" + dataElement + "/" + self.dataElementFiles[dataElement]
                                            
                                            # Transfer the file from the remote machine to the local path defined in previous step
                                            sftp.get(self.dataElementFiles[dataElement], localpath=localpath)
                                            self.logger.write("Received " + str(self.dataElementFiles[dataElement]))
                                
                                # Reset the "dataElementFiles" files dictionary 
                                self.dataElementFiles = {"spat" : None, "srm": None, "remoteBsm": None, "ssm": None, "msgCount": None}
                                
                                # Remove the data directory from the remote machine
                                sftp.rmdir(intersection["v2x-data_location"] + "/archive/" + directory)
                                self.logger.write("Removed remote path content " + str (intersection["v2x-data_location"] + "/archive/" + directory))
                    
                    # If the v2x-data/archive directory can not be found on the remote machine, print the error message to the console
                    except: self.logger.write("Failed to locate v2x-data archive on " + intersection["name"])
                
                # Else print on the console the success message
                self.logger("Data transfer from " + intersection["name"] + " completed. Logged out!")
            
            # If SFTP connection can not be established with the remote machine, print the error message on the console
            except: self.logger.write("Failed to establish SFTP connection with " + intersection["name"])

if __name__=="__main__":
    pass
            

