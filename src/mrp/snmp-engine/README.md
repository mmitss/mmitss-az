# SNMP-ENGINE
[Simple Network Management Protocol (SNMP)](https://en.wikipedia.org/wiki/Simple_Network_Management_Protocol) is one of widely accepted protocols to manage and monitor network elements. All NTCIP-1202 compliant traffic actuated signal controllers can be managed and monitored using the SNMP protocol.  

[Net-SNMP](http://www.net-snmp.org/) library provides C/C++ APIs that can be used to communicate with such devices over IPv4 or IPv6. A [tutorial](http://www.net-snmp.org/wiki/index.php/TUT:Simple_Application) on using the Net-SNMP libraries with C/C++ is available on the official website of Net-SNMP.  

The **Snmp-Engine** application builds upon the Net-SNMP library and provides simple JSON based APIs to monitor (through `get` requests) or manage (through `set` requests) the signal controllers complying to NTCIP-1202 standard.
