
# Message Distributor
- ``M_MessageDistributor.py`` is a wrapper module for the MessageDistributor class.
- This module receives BSMs from simulation and SRMs from priority-request-
generator(s) and based on the message type, calls appropriate methods of the 
MessageGenerator class to distribute the received message.
- The configuration file (JSON) for the corridor needs to be provided in the
command-line argument. 

- MessageDistributor class provides methods for distribution of BSMs and SRMs 
to multiple clients. The class supports the distribution of BSMs from four
vehicle types: (1) Transit, (2) Emergency, (3) Truck, and (4) Passenger
- BSMs are distributed to two types of clients: (1) Infrastructural
clients, and (2) additional BSM clients.
- PriorityRequestGenerator for different vehicle types could be one such 
additional BSM client.
- SRMs are distributed to infrastructural clients.
- For distributing the messages to infrastructural clients (intersections)
haversine distance between the center of the intersection and the vehicle's 
current location is calculated. If this distance is less than the DSRC range 
the intersection, then the message is distributed to that intersection.
- Clients of both types can be defined in the configuration file.

