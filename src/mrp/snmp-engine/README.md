# SNMP-ENGINE
The [Simple Network Management Protocol (SNMP)](https://en.wikipedia.org/wiki/Simple_Network_Management_Protocol) is one of widely accepted protocols to manage and monitor network elements. All [NTCIP-1202](https://www.ntcip.org/wp-content/uploads/2018/11/NTCIP1202v0219f.pdf) compliant traffic actuated signal controllers can be managed and monitored using the SNMP protocol.  

[Net-SNMP](http://www.net-snmp.org/) provides an open-source C/C++ library that has APIs to communicate with such devices over IPv4 or IPv6. A [tutorial](http://www.net-snmp.org/wiki/index.php/TUT:Simple_Application) on using the Net-SNMP libraries with C/C++ is available on the official [website](http://www.net-snmp.org/wiki/index.php/Main_Page) of Net-SNMP.  

The **Snmp-Engine** component of MMITSS builds upon the [Net-SNMP](http://www.net-snmp.org/wiki/index.php/Main_Page) library and provides simple JSON based APIs to monitor (through `get` requests) or manage (through `set` requests) the [NTCIP-1202](https://www.ntcip.org/wp-content/uploads/2018/11/NTCIP1202v0219f.pdf) compliant traffic actuated signal controllers.

## Work-flow
To monitor a particular SNMP object (having a defined OID) in the target SNMP device, a JSON formatted SnmpGet request can be sent to the Snmp-Engine as a UDP packet. An example of such JSON formatted SnmpGet request is as follows:
```
{
    "MsgType": "SnmpGetRequest",
    "OID": "1.3.6.1.4.1.1206.3.5.2.9.44.1.1"
}
```
After receiving such request, the Snmp-Engine forwards this request to the target SNMP device. After target SNMP device responds with the value of that OID, SnmpEngine formulates a JSON string containing the value of the requested OID and sends it back to the requestor's UDP socket. An example of the SnmpGetResponse (corresponding to above SnmpGet request) is as follows:
```
{
    "MsgType": "SnmpGetRequestResponse",
    "OID": "1.3.6.1.4.1.1206.3.5.2.9.44.1.1",
    "Value": 6
}
```

Similarly, a particular SNMP object (again, having a defined OID) in the target SNMP device can be managed by formulating and sending a JSON formatted SnmpSet request to the SnmpEngine. An example of such JSON formatted SnmpGet request is as follows:
```
{
    "MsgType": "SnmpSetRequest",
    "OID": "1.3.6.1.4.1.1206.3.5.2.9.44.1.1",
    "Value": 0
}
```

## Requirements
- Physical network connection between the host machine (hosting the SnmpEngine) and the target SNMP device ([NTCIP-1202](https://www.ntcip.org/wp-content/uploads/2018/11/NTCIP1202v0219f.pdf) compliant traffic actuated signal controller)

## Configuration

In the mmitss-phase3-master-config.json (config) file, configure the values of following keys:
- `config["SignalController"]["IpAddress"]`: IPv4 address of the signal controller (string)
- `config["SignalController"]["NtcipPort"]`: NTCIP Port of the signal controller (integer)
- `config["PortNumber"]["SnmpEngine"]`:  UDP port number on the host (integer). Note: Change only if the default (recommended) port number is already occupied on the host machine.



