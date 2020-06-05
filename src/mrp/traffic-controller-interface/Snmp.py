import socket
import json

class Snmp:
    def __init__(self):
        configFile = open("/nojournal/bin/mmitss-phase3-master-config.json", 'r')
        config = (json.load(configFile))
        # Close the config file:
        configFile.close()

        # Open a socket and bind it to the IP and port dedicated for this application:
        self.s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        self.s.bind((config["HostIp"], config["PortNumber"]["SnmpEngineInterface"]))

        self.snmpEngineCommInfo = (config["HostIp"],config["PortNumber"]["SnmpEngine"])

    def getValue(self, oid:str):
        
        snmpGetRequestJson = json.dumps({"MsgType":"SnmpGetRequest", "OID":oid})
        self.s.sendto(snmpGetRequestJson.encode(), self.snmpEngineCommInfo)
        data, address = self.s.recvfrom(1024)
        snmpGetResponseJson = json.loads(data.decode())
        
        return snmpGetResponseJson["Value"]

    def setValue(self, oid:str, value:int):
        snmpSetRequestJson = json.dumps({"MsgType":"SnmpSetRequest", "OID":oid, "Value":value})
        self.s.sendto(snmpSetRequestJson.encode(), self.snmpEngineCommInfo)
        

if __name__ == "__main__":
    snmp = Snmp()
    import time

    requestTime = time.time()
    print("Original value: " + str(snmp.getValue("1.3.6.1.4.1.1206.4.2.1.3.3.1")))
    deliveryTime = time.time()
    leadTime = deliveryTime - requestTime
    print("Received in " + str(leadTime) + " Seconds" )

    time.sleep(0.1)
    snmp.setValue("1.3.6.1.4.1.1206.4.2.1.3.3.1", 3)
    time.sleep(0.1)

    requestTime = time.time()
    print("New value: " + str(snmp.getValue("1.3.6.1.4.1.1206.4.2.1.3.3.1")))
    deliveryTime = time.time()
    leadTime = deliveryTime - requestTime
    print("Received in " + str(leadTime) + " Seconds" )

    
