# Import system libraries:
import os
import datetime
import json

# Import 3rd party libraries:
import pysftp

# Import local modules
from V2XDataTransfer import V2XDataTransfer

class PullFromIntersection(V2XDataTransfer):
    def __init__(self, serverDataDirectory:str, intersectionList:str):
        super().__init__(serverDataDirectory, intersectionList)
        self.verify_or_create_local_directory_structure()

    def verify_or_create_local_directory_structure(self):
        # Check if directory structure exists for listed intersections. If not, create the required folders:
        for intersection in self.intersectionList:
            intersectionDirectory = self.serverDataDirectory + "/" + intersection["name"]
            for dataElement in self.dataElements:
                dataElementDirectory = intersectionDirectory + "/" + dataElement 
                if not os.path.isdir(dataElementDirectory):
                    os.makedirs(dataElementDirectory)

    def transfer_data(self):
        # For each intersection (remote machine):
        for intersection in self.intersectionList:
            try:
                # Establish an SFTP connection
                with pysftp.Connection(intersection["ip_address"], username=intersection["username"], password=intersection["password"]) as sftp:
                    try:
                        # On remote machine, change the working directory to v2x-data/archive
                        with sftp.cd(intersection["v2x-data_location"] + "/archive/"):
                            # List archived directories
                            remoteArchivedDirectories = sftp.listdir()
                            
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
                                
                                # Reset the "dataElementFiles" files dictionary 
                                self.dataElementFiles = {"spat" : None, "srm": None, "remoteBsm": None, "ssm": None, "msgCount": None}
                                # Remove the data directory from the remote machine
                                sftp.rmdir(intersection["v2x-data_location"] + "/archive/" + directory)
                    
                    # If the v2x-data/archive directory can not be found on the remote machine, print the error message to the console
                    except: print("Failed to locate v2x-data archive on " + intersection["name"] + " at:" + str(datetime.datetime.now()))
                
                # Else print on the console the success message
                print("Data transfer from " + str(intersection["name"]) + " completed at: " + str(datetime.datetime.now()))
            
            # If SFTP connection can not be established with the remote machine, print the error message on the console
            except: print("Failed to establish SFTP connection with " + intersection["name"] + " at: " + str(datetime.datetime.now()))

if __name__=="__main__":
    """
    UNIT TEST MENU:
    1. Verify or create the directory structure:
    """
    TESTS = [1]

    configFilename = "test/v2x-data-transfer-config.json"
    with open(configFilename, 'r') as configFile:
        config = json.load(configFile)

    pullFromIntersection = PullFromIntersection(config["server"], config["intersections"])

    for testId in TESTS:
        if testId == 1:
            try:
                pullFromIntersection.verify_or_create_local_directory_structure()
                print("Test#" + str(testId) + " successful!")
            except:
                print("Test#" + str(testId) + " failed!")
            

