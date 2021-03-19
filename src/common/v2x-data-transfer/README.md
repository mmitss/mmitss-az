# V2X-Data-Transfer
The V2X-Data-Transfer application facilitates the transfer of data files between intersections, local data servers and [CyVerse](https://cyverse.org/about). This application can either be hosted on intersection coprocessors or on the local data server.

## Work-flow
When hosted on the intersection coprocessor, the V2X-Data-Collector can push the data files to the server using [SFTP](https://www.ssh.com/ssh/sftp/), every day at a configured time (HH:MM), and then delete the local copy. On the other hand, if hosted on the local server (that has connectivity to the internet as well as intersections), V2X-Data-Collector can perform following tasks:
1. Pull data files from multiple intersections and store locally, using SFTP, every day at a configured time (HH:MM) - and delete the copy stored on intersections.
2. Push the locally stored data files to CyVerse, using [iCommands](https://learning.cyverse.org/projects/data_store_guide/en/latest/step2.html), every day at a defined time (HH:MM) - and delete the copy stored on the server (Supported on Linux-based platforms only). 

In any case, the data transfer event is scheduled to be executed every day at time configured in the configuration file. For some unpredicted reason (e.g. unavailability of network connection, or loss of connection when the transfer is in progress), if the data transfer process fails, or can not be started for the day, then another attempt will be made the next day. In such cases files can remain on the source device, and in next the attempt (if able), files for the previous day(s) are transferred too.

# Console output and logging
The V2X-Data-Collector provides console output indicating success or failure of each data transfer event. It does not produce any log files.

# Configuration

# Requirements
1. Common to both, serverside and roadside deployment:
  - Operating System: Linux or Windows (limited functionality)
  - Python3.6+
  - Python packages: 
    - [sh](https://pypi.org/project/sh/): install using `pip3 install sh` 
    - [apscheduler](https://pypi.org/project/APScheduler/): install using `pip3 install apscheduler`
    - [pysftp](https://pypi.org/project/pysftp/): install using `pip3 install pysftp`
  - Other linux packages (N/A for Windows):
    - [iCommands](https://learning.cyverse.org/projects/data_store_guide/en/latest/step2.html)
3. Serverside deployment:
  - Minimally disrupted network connectivity with intersections
  - Minimally disrupted network connectivity with internet
  

# Known issues/limitations
