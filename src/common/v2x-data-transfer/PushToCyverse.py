"""
**********************************************************************************

 © 2021 Arizona Board of Regents on behalf of the University of Arizona with rights
       granted for USDOT OSADP distribution with the Apache 2.0 open source license.

**********************************************************************************

  PushToCyverse.py  
  Created by: Niraj Vasant Altekar
  University of Arizona   
  College of Engineering

  This code was developed under the supervision of Professor Larry Head
  in the Systems and Industrial Engineering Department.

**********************************************************************************
"""

# Import system libraries:
import os
import json

# Import 3rd party libraries:
import sh

# Import local modules:
from V2XDataTransfer import V2XDataTransfer
from Logger import Logger

class PushToCyverse(V2XDataTransfer):
    """
    provides a method to transfer the data from from server to CyVerse.
    This method is intended to be deployed on the server that has
    connectivity to both: intersections as well as internet.

    NOTE: This class requires `icommands` utility installed on the 
    host machine. As this utility is available only on Linux/Mac platforms,
    this class can only be deployed on Linux/Mac platforms.

    More on `icommands`: https://learning.cyverse.org/projects/data_store_guide/en/latest/step2.html 
    """

    def __init__(self, serverDataDirectory:str, intersectionList:str, logger:Logger):
        
        # Initialize the parent class
        super().__init__(serverDataDirectory, intersectionList, logger)

    def transfer_data(self):
        """
        for each intersection whose data is stored on the host machine, transfers the
        collected data to cyverse at a defined location
        """
        
        # For each intersection:
        for intersection in self.intersectionList:

            # For each data element:
            for dataElement in self.dataElements:

                # Get local path of the data directory pertaining to the data element
                localDirectory = self.serverDataDirectory + "/" + intersection["name"] + "/" + dataElement
                
                # Get cyverse path of the data directory pertaining to the data element of the intersection
                cyverseDirectory = intersection["cyverse_location"] + "/" + dataElement

                # Transfer the entire content of the directory and remove the local copy if the transfer is successful
                self.transfer_directory_content(localDirectory, cyverseDirectory)

    def transfer_directory_content(self, localDirectory:str, cyverseDirectory:str):
        """
        Transfers the content from the localDirectory to the cyverseDirectory - Needs internet connection.
        """
        try:
            # Change the working directory to the local directory FROM where data needs to be transferred
            sh.cd(localDirectory)

            # On icommands, change the working directory to the CyVerse directory TO where the data needs to be transferred
            sh.icd(cyverseDirectory)

            # Upload the data to CyVerse
            sh.iput("-r","-f",".")
            self.logger.write("Content of " + localDirectory + " is transfered to " + cyverseDirectory)

            # Change the local working directory to the original working directory
            sh.cd(self.workingDirectory)

            # Remove the local directory FROM where the data is already uploaded in earlier steps
            sh.rm("-r", localDirectory)
            self.logger.write("Removed local content from " + localDirectory)

            # Reset the directory structure
            os.makedirs(localDirectory)

            

        except Exception as e:

            # If something goes wrong, print the message on the console
            self.logger.write(str(e))
            self.logger.write(self.logger.write("Data transfer unsuccessful from " + localDirectory + " to " + cyverseDirectory))

if __name__ == "__main__":
    pass

