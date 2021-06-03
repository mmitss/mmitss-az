"""
**********************************************************************************

 © 2021 Arizona Board of Regents on behalf of the University of Arizona with rights
       granted for USDOT OSADP distribution with the Apache 2.0 open source license.

**********************************************************************************

  PushToServer.py  
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
import shutil

# Import 3rd party libraries:
import pysftp

# Import local modules
from V2XDataTransfer import V2XDataTransfer
from Logger import Logger

class PushToServer(V2XDataTransfer):
    """
    provides method for transfering the data from the intersection to the server.
    This class is intended to be deployed on intersections.
    """
    def __init__(self, server:dict, intersectionList:str, logger:Logger):
        
        # Initialize the parent class
        super().__init__(server, intersectionList, logger)
        
        # Store the configuration information
        self.name = self.intersectionList[0]["name"]
        self.v2xDataLocation = self.intersectionList[0]["v2x-data_location"]
        self.serverIpAddress = server["ip_address"]
        self.serverUsername = server["username"]
        self.serverPassword = server["password"]

        # Disable host key verification
        self.cnopts = pysftp.CnOpts()
        self.cnopts.hostkeys = None 

        # Create the directory structure on the remote machine if it does not exist:
        self.verify_or_create_remote_directory_structure()


    def verify_or_create_remote_directory_structure(self):
        """
        verifies if the correct directory structure is available on the server to store files 
        pertaining to all data elements. The directory structure would look like the following:
        - RootDataDirectory
            - Intersection-1
                - msgCount
                - remoteBsm
                - spat
                - srm
                - ssm
        """

        # Get the location of the data directory on the remote machine pertaining to the intection
        # using the information obtained from the configuration
        intersectionDirectory = self.serverDataDirectory + "/" + self.name
        
        try:
            # Establish an SFTP connection
            with pysftp.Connection(self.serverIpAddress, username=self.serverUsername, password=self.serverPassword, cnopts=self.cnopts) as sftp:
                self.logger.write("Logged in to server. IP address:" + self.serverIpAddress)
                
                # For each data element:
                for dataElement in self.dataElements:
                    
                    # Create the name of the data directory:
                    dataElementDirectory = intersectionDirectory + "/" + dataElement
                    
                    # If the directory does not exists:
                    if not sftp.exists(dataElementDirectory):
                        
                        # Create the directory on the local machine (alllows recursion)
                        sftp.makedirs(dataElementDirectory)
                        self.logger.write("Created remote path " + dataElementDirectory)

        except Exception as e:
            # If something is unsuccessful, print the message to the console
            print(e)


    def transfer_data(self):
        """
        from each of the data directories archived on the host machine, transfers the files to their 
        respective remote paths using SFTP.

        NOTE: Host key verification is disabled here -> could be seen as a security risk
        """

        # Formulate the archive directory path
        dataArchivePath = self.v2xDataLocation + "/archive"
        
        # Change the working directory to the archive directory
        os.chdir(dataArchivePath)

        # Get the list of all archived directories
        localArchivedDirectories = os.listdir()
        try:
            # Establish an SFTP connection
            with pysftp.Connection(self.serverIpAddress, username=self.serverUsername, password=self.serverPassword, cnopts=self.cnopts) as sftp:
                
                # For each archived directory on the host machine:
                for directory in localArchivedDirectories:
                    try:
                        # On local machine, change the working directory to v2x-data/archive/archivedDirectory
                        os.chdir(directory)

                        # List all files available in the directory
                        dataFiles = os.listdir()
                            
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
                                # Set the remote path where file needs to be transferred:
                                remotepath=self.serverDataDirectory + "/" + self.name + "/" + dataElement + "/" + self.dataElementFiles[dataElement]
                                # Transfer the file from the host machine to the remote path defined in previous step
                                sftp.put(self.dataElementFiles[dataElement],remotepath=remotepath)
                    
                        # Reset the "dataElementFiles" dictionary 
                        self.dataElementFiles = {"spat" : None, "srm": None, "remoteBsm": None, "ssm": None, "msgCount": None}
                    
                        # Change working directory to original working directory:
                        os.chdir(self.workingDirectory)

                        # Remove the data directory from the host machine
                        shutil.rmtree((self.v2xDataLocation + "/archive/" + directory))
        
                    # If the v2x-data/archive/directory can not be found on the host machine, print the error message to the console
                    except Exception as e: 
                        print("Failed to transfer data from " + directory+ " at:" + str(datetime.datetime.now()))    
                        print(e)
                # Else print on the console the success message
                print("Data transfer from " + self.name + " completed at: " + str(datetime.datetime.now()))

        # If SFTP connection can not be established with the remote machine, print the error message on the console
        except: print("Failed to establish SFTP connection with server " + self.serverIpAddress + " at: " + str(datetime.datetime.now()))

if __name__ == "__main__":
    pass