"""
***************************************************************************************

 © 2019 Arizona Board of Regents on behalf of the University of Arizona with rights
       granted for USDOT OSADP distribution with the Apache 2.0 open source license.

***************************************************************************************

Command.py
Created by: Niraj Vasant Altekar
University of Arizona   
College of Engineering

This code was developed under the supervision of Professor Larry Head
in the Systems and Industrial Engineering Department.

***************************************************************************************

Description:
------------
This class provides a data-structure to store the commands that are received from the 
MMITSS components, which need to be forwarded to the traffic signal controller. 
The class also defines the action types that can be forwarded to the signal controller.
This class has NO member functions 

***************************************************************************************
"""

class Command:
    """
    This class provides a data-structure to store the commands which can be sent to the 
    through other modules in TCI. To instantiate a new object of this class, four
    arguments are required: (1) affected phases (bitstring to integer), (2) action I, 
    (3) startTime from now in seconds, and (4) endTime from now in seconds.

    The action IDs are defined in the class itself.

    For example, for placing a vehicle call on ALL (11111111) phases, from 20 seconds 
    from now until 21 seconds from now,
    - > command = Command(255, 1, 20, 21)
    """
    def __init__(self, phases:int, action:int, startTime:float, endTime:float):

        self.action = action
        self.phases = phases
        self.startTime = startTime
        self.endTime = endTime

        # Define Command Actions (Used in phase controls):
        self.CALL_VEH_PHASES = "call_veh"
        self.CALL_PED_PHASES = "call_ped"
        self.FORCEOFF_PHASES = "forceoff"
        self.HOLD_VEH_PHASES = "hold"
        self.OMIT_VEH_PHASES = "omit_veh"
        self.OMIT_PED_PHASES = "omit_ped"

'''##############################################
                   Unit testing
##############################################'''
if __name__ == "__main__":
    command = Command(255, 1, 20, 21)