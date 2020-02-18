from pysnmp.hlapi import *

class SnmpApi:
    def __init__(self, targetDeviceCommInfo:tuple):
        self.targetDeviceCommInfo = targetDeviceCommInfo

    def snmpGet(self, oid:str):
        errorIndication, errorStatus, errorIndex, varBinds = next(getCmd(
                                                                        SnmpEngine(),
                                                                        CommunityData('public', mpModel=0),
                                                                        UdpTransportTarget(self.targetDeviceCommInfo),
                                                                        ContextData(),
                                                                        ObjectType(ObjectIdentity(oid))
                                                                        )
                                                                )

        if errorIndication:
            result = (errorIndication)
            return result
        elif errorStatus:
            result = ('%s at %s' % (errorStatus.prettyPrint(),
                                errorIndex and varBinds[int(errorIndex) - 1][0] or '?'))
            return result
        else:
            result = varBinds[0][1].prettyPrint()
            return result
            
    def snmpSet(self, oid:str, value:int):
        errorIndication, errorStatus, errorIndex, varBinds = next(setCmd(
                                                                        SnmpEngine(),
                                                                        CommunityData('public', mpModel=0),
                                                                        UdpTransportTarget(self.targetDeviceCommInfo),
                                                                        ContextData(),
                                                                        ObjectType(ObjectIdentity(oid), Integer(value))
                                                                        )
                                                                )

        if errorIndication:
            result = errorIndication
            return result
        elif errorStatus:
            result = ('%s at %s' % (errorStatus.prettyPrint(),
                                errorIndex and varBinds[int(errorIndex) - 1][0] or '?'))
            return result
        else:
            result = 0
            return True

if __name__ == "__main__":
    controllerIp = "10.12.6.107"
    controllerPort = 501
    controllerCommInfo = (controllerIp, controllerPort)
    controllerSnmpApi = SnmpApi(controllerCommInfo)
    print(controllerSnmpApi.snmpGet("1.3.6.1.4.1.1206.3.5.2.9.44.1.1"))
    