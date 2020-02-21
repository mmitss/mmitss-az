from easysnmp import Session

class SnmpApi:
    def __init__(self, targetDeviceCommInfo:tuple):
        self.targetDeviceIp, self.targetDevicePort = targetDeviceCommInfo
        self.session = Session(hostname=self.targetDeviceIp, community="public", version=1, remote_port=self.targetDevicePort)

    def snmpGet(self, oid:str):
        result = self.session.get(oid).value
        return result

    def snmpSet(self, oid:str, value:int):
        self.session.set(oid, value, "int")

if __name__ == "__main__":
    controllerIp = "10.12.6.107"
    controllerPort = 501
    controllerCommInfo = (controllerIp, controllerPort)
    controllerSnmpApi = SnmpApi(controllerCommInfo)
    print(controllerSnmpApi.snmpGet("1.3.6.1.4.1.1206.3.5.2.9.44.1.1"))
    