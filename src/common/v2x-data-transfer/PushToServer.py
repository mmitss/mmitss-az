# Import system libraries:
import os
import datetime
import json
import shutil

# Import 3rd party libraries:
import pysftp

# Import local modules
from V2XDataTransfer import V2XDataTransfer

class PushToServer(V2XDataTransfer):
    def __init__(self, server:dict, intersectionList:str):
        super().__init__(server, intersectionList)
        self.name = self.intersectionList[0]["name"]
        self.v2xDataLocation = self.intersectionList[0]["v2x-data_location"]
        self.serverIpAddress = server["ip_address"]
        self.serverUsername = server["username"]
        self.serverPassword = server["password"]
        self.cnopts = pysftp.CnOpts()
        self.cnopts.hostkeys = None 
        self.verify_or_create_remote_directory_structure()


    def verify_or_create_remote_directory_structure(self):
        intersectionDirectory = self.serverDataDirectory + "/" + self.name
        # Establish an SFTP connection
        try:
            with pysftp.Connection(self.serverIpAddress, username=self.serverUsername, password=self.serverPassword, cnopts=self.cnopts) as sftp:
                for dataElement in self.dataElements:
                    dataElementDirectory = intersectionDirectory + "/" + dataElement
                    if not sftp.exists(dataElementDirectory):
                        sftp.makedirs(dataElementDirectory)
        except Exception as e:
            print(e)


    def transfer_data(self):
        dataArchivePath = self.v2xDataLocation + "/archive"
        os.chdir(dataArchivePath)

        localArchivedDirectories = os.listdir()
        try:
            # Establish an SFTP connection
            with pysftp.Connection(self.serverIpAddress, username=self.serverUsername, password=self.serverPassword, cnopts=self.cnopts) as sftp:
                # For each archived directory:
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
                                # Transfer the file from the remote machine to the local path defined in previous step
                                sftp.put(self.dataElementFiles[dataElement],remotepath=remotepath)
                    
                        # Reset the "dataElementFiles" files dictionary 
                        self.dataElementFiles = {"spat" : None, "srm": None, "remoteBsm": None, "ssm": None, "msgCount": None}
                    
                        # Change working directory to original working directory:
                        os.chdir(self.workingDirectory)

                        # Remove the data directory from the remote machine
                        shutil.rmtree((self.v2xDataLocation + "/archive/" + directory))
        
                    # If the v2x-data/archive directory can not be found on the remote machine, print the error message to the console
                    except Exception as e: 
                        print("Failed to transfer data from " + directory+ " at:" + str(datetime.datetime.now()))    
                        print(e)
                # Else print on the console the success message
                print("Data transfer from " + self.name + " completed at: " + str(datetime.datetime.now()))

        # If SFTP connection can not be established with the remote machine, print the error message on the console
        except: print("Failed to establish SFTP connection with server " + self.serverIpAddress + " at: " + str(datetime.datetime.now()))

if __name__ == "__main__":
    configFilename = "test/v2x-data-transfer-config.json"
    with open(configFilename, 'r') as configFile:
        config = json.load(configFile)

    pushToServer = PushToServer(config["server"], config["intersections"])
    pushToServer.transfer_data()