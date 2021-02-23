# Import system libraries
from abc import ABC, abstractmethod

# Import third party libraries
import sh

class V2XDataTransfer(ABC):
    """
    an abstract class defining a structure for inherited classes, where each child class would be responsinble
    for performing certain kind of data transfer
    """
    def __init__(self, server:dict, intersectionList:list):
        self.serverDataDirectory = server["data_directory"]
        self.intersectionList = intersectionList
        self.dataElements = ["spat", "srm", "remoteBsm", "ssm", "msgCount"]
        self.dataElementFiles = {"spat" : None, "srm": None, "remoteBsm": None, "ssm": None, "msgCount": None}
        self.workingDirectory = str(sh.pwd())[:-1]

    @abstractmethod
    def transfer_data(self):
        """
        this method will depend on the desired type of data transfer
        """
        pass

if __name__=="__main__":
    pass