# TRAFFIC-CONTROLLER-INTERFACE

## Work-flow

## Console output and logging
Traffic-Controller-Interface component does not generate any log files. The console output displays if the broadcast of the Map and SPaT messages has started successfully.

## Requirements
- The Snmp-Engine component of MMITSS forwards the `get` or `set` requests formulated by the Traffic-Controller-Interface to the signal controller. Therefore, for correct functioning of the Traffic-Controller-Interface, Snmp-Engine needs to be started before the Traffic-Controller-Interface.
- Python package APScheduler needs to be installed. For Python3, it can be installed via `pip3 install apscheduler`.

## Configuration
1. In the signal controller:
  - NTCIP backup time must be > 0

2. In the `mmitss-phase3-master-config.json` (config) file, following keys need to be assigned with appropriate values:


## Known issues/limitations
- None -
