"""
***************************************************************************************

 © 2019 Arizona Board of Regents on behalf of the University of Arizona with rights
       granted for USDOT OSADP distribution with the Apache 2.0 open source license.

***************************************************************************************

Snmp.py
Created by: Niraj Vasant Altekar
University of Arizona   
College of Engineering

This code was developed under the supervision of Professor Larry Head
in the Systems and Industrial Engineering Department.

***************************************************************************************

Description:
------------
This class provides an API for getting and setting the values for OID, through SNMP session.

easysnmp library needs to be installed on the host machine through pip, after installing the 
original NetSNMP base libraries.

For downloading base NetSNMP libraries: http://net-snmp.sourceforge.net/download.html
For installing base NetSNMP libraries: http://net-snmp.sourceforge.net/download.html

It is preferred to compile NetSNMP from the source, as the packages available through "apt" 
are often archaic.

Finally, for installing easysnmp libraries: https://easysnmp.readthedocs.io/en/latest/

***************************************************************************************
"""

# Import the Session class from the easysnmp library. 
from easysnmp import Session

class Snmp:
    """
    Snmp class provides easy API (for get and set) to the library used for SNMP sessions. 
    A single SNMP session is opened in the constructor of Snmp class and it is reused in further calls to SNMP device.
    A tuple containing the IP address (string) and NTCIP port (integer) of the target SNMP device needs to be provided as an argument for instantiating an object of this class.
    For example: snmpObject = Snmp(("10.12.6.10", 501))
    """
    def __init__(self, targetDeviceCommInfo:tuple):
        self.targetDeviceCommInfo = targetDeviceCommInfo
        self.targetDeviceIp, self.targetDevicePort = targetDeviceCommInfo
        self.session = Session(hostname=self.targetDeviceIp, community="public", version=1, remote_port=self.targetDevicePort)
    ######################## Definition End: __init__(self, targetDeviceCommInfo:tuple) ########################

    def getValue(self, oid:str):
        """
        Snmp::getValue function takes an OID as an argument. Through the established session this function queries the SNMP device for the requested value. 
        Finally the function returns the value received from the SNMP device for the requested OID.
        """
        value = self.session.get(oid).value
        return value
    ######################## Definition End: getValue(self, oid:str) ########################


    def setValue(self, oid:str, value:int):
        """
        Snmp::setValue function takes two arguments: (1) an OID for which the value needs to be set, and (2) the value.
        The function sets the requested value to the requested OID through the established SNMP session.
        """
        self.session.set(oid, value, "int")
    ######################## Definition End: setValue(self, oid:str) ########################


'''##############################################
                   Unit testing
##############################################'''
if __name__ == "__main__":
    signalControllerIP = "10.12.6.17"
    signalControllerPort = 501
    signalControllerCommInfo = (signalControllerIP, signalControllerPort)
    snmp = Snmp(signalControllerCommInfo)