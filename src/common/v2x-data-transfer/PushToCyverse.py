# Import system libraries:
import os
import json

# Import 3rd party libraries:
import sh

# Import local modules:
from V2XDataTransfer import V2XDataTransfer

class PushToCyverse(V2XDataTransfer):
    def __init__(self, serverDataDirectory:str, intersectionList:str):
        super().__init__(serverDataDirectory, intersectionList)

    def transfer_data(self):
        for intersection in self.intersectionList:
            for dataElement in self.dataElements:
                localDirectory = self.serverDataDirectory + "/" + intersection["name"] + "/" + dataElement
                cyverseDirectory = intersection["cyverse_location"] + "/" + dataElement
                self.transfer_directory_content(localDirectory, cyverseDirectory)

    def transfer_directory_content(self, localDirectory:str, cyverseDirectory:str):
        """
        Transfers the content from the localDirectory to the cyverseDirectory - Needs internet connection.
        """
        try:
            sh.cd(localDirectory)
            sh.icd(cyverseDirectory)
            sh.iput("-r","-f",".")
            sh.cd(self.workingDirectory)
            sh.rm("-r", localDirectory)
            os.makedirs(localDirectory)
        except Exception as e:
            print(e)

if __name__ == "__main__":

    configFilename = "test/v2x-data-transfer-config.json"
    with open(configFilename, 'r') as configFile:
        config = json.load(configFile)

    pushToCyverse = PushToCyverse(config["server"], config["intersections"])
    pushToCyverse.transfer_data()


