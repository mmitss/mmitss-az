"""
**********************************************************************************

 © 2021 Arizona Board of Regents on behalf of the University of Arizona with rights
       granted for USDOT OSADP distribution with the Apache 2.0 open source license.

**********************************************************************************

  V2XDataTransfer.py  
  Created by: Niraj Vasant Altekar
  University of Arizona   
  College of Engineering

  This code was developed under the supervision of Professor Larry Head
  in the Systems and Industrial Engineering Department.

**********************************************************************************
"""

# Import system libraries
from abc import ABC, abstractmethod

# Import third party libraries
import sh

# Import local classes
from Logger import Logger

class V2XDataTransfer(ABC):
    """
    an abstract class defining a structure for inherited classes, where each child class would be responsinble
    for performing certain kind of data transfer
    """
    def __init__(self, server:dict, intersectionList:list, logger:Logger):
        self.logger = logger
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