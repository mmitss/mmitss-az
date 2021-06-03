# SNMP-ENGINE
In MMITSS ecosystem, Snmp-Engine is the **only** application that communicates directly with the signal controller through using SNMP. The [Simple Network Management Protocol (SNMP)](https://en.wikipedia.org/wiki/Simple_Network_Management_Protocol) is one of widely accepted protocols to manage and monitor network elements. All [NTCIP-1202](https://www.ntcip.org/wp-content/uploads/2018/11/NTCIP1202v0219f.pdf) compliant traffic actuated signal controllers can be managed and monitored using the SNMP protocol.  

[Net-SNMP](http://www.net-snmp.org/) provides an open-source C/C++ library that has APIs to communicate with such devices over IPv4 or IPv6. A [tutorial](http://www.net-snmp.org/wiki/index.php/TUT:Simple_Application) on using the Net-SNMP libraries with C/C++ is available on the official [website](http://www.net-snmp.org/wiki/index.php/Main_Page) of Net-SNMP.  

The **Snmp-Engine** component of MMITSS builds upon the [Net-SNMP](http://www.net-snmp.org/wiki/index.php/Main_Page) library and provides simple JSON based APIs to monitor (through `get` requests) or manage (through `set` requests) the [NTCIP-1202](https://www.ntcip.org/wp-content/uploads/2018/11/NTCIP1202v0219f.pdf) compliant traffic actuated signal controllers. 

NOTE: In case if it is required to run or monitor MMITSS applications without controlling the intersection signal controller, the Snmp-Engine application can be turned off.

## Work-flow
The Snmp-Engine component establishes and maintains a single SNMP session with the target SNMP device. This session is reused for all `get` and `set` requests. Post opening the SNMP session, the components waits for UDP packets containing JSON formatted `get` or `set` requests.

### Monitoring the target SNMP device through `get` requests
To monitor a particular SNMP object (having a defined OID) in the target SNMP device, a JSON formatted SnmpGetRequest can be sent to the Snmp-Engine as a UDP packet. An example of such JSON formatted SnmpGetRequest is as follows:
```
{
    "MsgType": "SnmpGetRequest",
    "OID": "1.3.6.1.4.1.1206.3.5.2.9.44.1.1"
}
```
Upon receiving such request, the Snmp-Engine forwards this request to the target SNMP device. After the target SNMP device responds with the value corresponding to the questioned OID, Snmp-Engine formulates a JSON string containing the value of the requested OID and sends it back to the **requestor's UDP socket**. An example of the SnmpGetRequestResponse (corresponding to above SnmpGetRequest) is as follows:
```
{
    "MsgType": "SnmpGetRequestResponse",
    "OID": "1.3.6.1.4.1.1206.3.5.2.9.44.1.1",
    "Value": 6
}
```
### Managing the target SNMP device through `set` requests
A particular SNMP object (again, having a defined OID) in the target SNMP device can be managed by formulating and sending a JSON formatted SnmpSet request to the SnmpEngine. An example of such JSON formatted SnmpGetRequest is as follows:
```
{
    "MsgType": "SnmpSetRequest",
    "OID": "1.3.6.1.4.1.1206.3.5.2.9.44.1.1",
    "Value": 0
}
```

## Console output and logging
If `["Logging"]` key is set to `true` in the `mmitss-phase3-master-config.json` configuration file, the SnmpEngine generates log file in that stores the information detailed in following bullet points. This information is also displayed on the console if `["Console"]` key is set to `true` in the `mmitss-phase3-master-config.json` configuration file
- Status of network connection with the target SNMP device. If the target SNMP device is not reachable in the network, the Snmp-Engine component gracefully exits.
- SUCCESS or FAILURE of each SnmpSetRequest and SnmpGetRequest with a unix timestamp of execution.

## Requirements
- Physical network connection between the host machine (hosting the SnmpEngine) and the target SNMP device ([NTCIP-1202](https://www.ntcip.org/wp-content/uploads/2018/11/NTCIP1202v0219f.pdf) compliant traffic actuated signal controller)

## Configuration

In the `mmitss-phase3-master-config.json` (config) file following keys need to be assigned with appropriate values:
- `config["SignalController"]["IpAddress"]`: IPv4 address of the signal controller (string)
- `config["SignalController"]["NtcipPort"]`: NTCIP Port of the signal controller (integer)
- `config["PortNumber"]["SnmpEngine"]`:  UDP port number on the host (integer). Note: Change only if the default (recommended) port number is already occupied on the host machine.

## Known issues

The MMITSS distribution is bundled with precompiled minimal Net-Snmp library, which excludes standard MIBs. Therefore, at the start of the application, following warnings are generated at the runtime:

```
MIB search path: /home/<user>/.snmp/mibs:/usr/local/share/snmp/mibs
Cannot find module (SNMPv2-MIB): At line 0 in (none)
Cannot find module (IF-MIB): At line 0 in (none)
Cannot find module (IP-MIB): At line 0 in (none)
Cannot find module (TCP-MIB): At line 0 in (none)
Cannot find module (UDP-MIB): At line 0 in (none)
Cannot find module (HOST-RESOURCES-MIB): At line 0 in (none)
Cannot find module (NOTIFICATION-LOG-MIB): At line 0 in (none)
Cannot find module (DISMAN-EVENT-MIB): At line 0 in (none)
Cannot find module (DISMAN-SCHEDULE-MIB): At line 0 in (none)
Cannot find module (UCD-SNMP-MIB): At line 0 in (none)
Cannot find module (UCD-DEMO-MIB): At line 0 in (none)
Cannot find module (SNMP-TARGET-MIB): At line 0 in (none)
Cannot find module (NET-SNMP-AGENT-MIB): At line 0 in (none)
Cannot find module (HOST-RESOURCES-TYPES): At line 0 in (none)
Cannot find module (SNMP-FRAMEWORK-MIB): At line 0 in (none)
Cannot find module (SNMP-MPD-MIB): At line 0 in (none)
Cannot find module (SNMP-USER-BASED-SM-MIB): At line 0 in (none)
Cannot find module (SNMP-VIEW-BASED-ACM-MIB): At line 0 in (none)
Cannot find module (SNMP-COMMUNITY-MIB): At line 0 in (none)
Cannot find module (IPV6-ICMP-MIB): At line 0 in (none)
Cannot find module (IPV6-MIB): At line 0 in (none)
Cannot find module (IPV6-TCP-MIB): At line 0 in (none)
Cannot find module (IPV6-UDP-MIB): At line 0 in (none)
Cannot find module (IP-FORWARD-MIB): At line 0 in (none)
Cannot find module (NET-SNMP-PASS-MIB): At line 0 in (none)
Cannot find module (NET-SNMP-EXTEND-MIB): At line 0 in (none)
Cannot find module (UCD-DLMOD-MIB): At line 0 in (none)
Cannot find module (SNMP-NOTIFICATION-MIB): At line 0 in (none)
Cannot find module (SNMPv2-TM): At line 0 in (none)
Cannot find module (NET-SNMP-VACM-MIB): At line 0 in (none)
```

These warnings do not affect the working of this component, as Snmp-Engine does not use any of the MIBs that are not available. If one wishes to suppress these warnings, the environment variable `MIBS` can be set to `ALL`. For current session, this can be accomplished by entering the following command in the terminal before starting the application or for persistence the following line can be added to the `~/.bashrc` file:  
```export MIBS=ALL```

