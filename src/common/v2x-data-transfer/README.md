# V2X-Data-Transfer
The V2X-Data-Transfer application facilitates the transfer of data files between intersections, local data servers and [CyVerse](https://cyverse.org/about). This application can either be hosted on intersection coprocessors or on the local data server.

## Work-flow
When hosted on the intersection coprocessor, the V2X-Data-Collector can push the data files to the server using [SFTP](https://www.ssh.com/ssh/sftp/), every day at a configured time (HH:MM), and then delete the local copy. On the other hand, if hosted on the local server (that has connectivity to the internet as well as intersections), V2X-Data-Collector can perform following tasks:
1. Pull data files from multiple intersections sequentially and store them locally, using SFTP, every day at a configured time (HH:MM) - and delete the copy stored on intersections.
2. Push the locally stored data files to CyVerse (sequentially for each intersection), using [iCommands](https://learning.cyverse.org/projects/data_store_guide/en/latest/step2.html), every day at a defined time (HH:MM) - and delete the copy stored on the server (Supported on Linux-based platforms only). 

In any case, the data transfer event is scheduled to be executed every day at time configured in the configuration file. For some unpredicted reason (e.g. unavailability of network connection, or loss of connection when the transfer is in progress), if the data transfer process fails, or can not be started for the day, then another attempt will be made the next day. In such cases files can remain on the source device, and in next the attempt (if able), files for the previous day(s) are transferred too.

# Console output and logging
The V2X-Data-Collector provides console output indicating success or failure of each data transfer event. It does not produce any log files.

# Configuration
The following fields are required to be configured in the `/nojournal/bin/mmitss-phase3-master-config.json` file:
1. Common to both, serverside and roadside deployment:
  - `["ApplicationPlatform"]`: a `string` specifying the platform where the application is running. Supported platforms: `roadside` and `server`
  - `["DataTransfer"]["server"]["data_directory"]`: a `string` specifying the absolute location of the data directory on the server
  - `["DataTransfer"]["intersection"]`: a `list` in which each element contains information about one intersection. For roadside deployment, this list should have only single element, consisting of information about the `self`. For server deployment, the number of elements in this list would be equal to the number of intersections in the corridor from which the data needs to be pulled.
  
2. Roadside deployment only:  
  - `["DataTransfer"]["server"]["ip_address"]`: a `string` specifying the IPv4 address of the local data server
  - `["DataTransfer"]["server"]["username"]`: a `string` specifying the username that is allowed SFTP transactions with the server
  - `["DataTransfer"]["server"]["password"]`: a `string` specifying the password corresponding to the username that is allowed SFTP transactions with the server
  - `["DataTransfer"]["StartTime_PushToServer"]["hour"]`: an `int` specifying the hour component of the time at which an attempt to **push the data to the server** must begin (range: 0-23) 
  - `["DataTransfer"]["StartTime_PushToServer"]["minute"]`: an `int` specifying the minute component of the time at which attempt to **push the data to the server** must begin (range: 0-59)
  - `["DataTransfer"]["intersection"][0]["name"]`: a `string` specifying the name of the intersection (self)
  - `["DataTransfer"]["intersection"][0]["v2x-data_location"]`: a `string` specifying the absolute location of the v2x-data directory (in the host file system), which consists the data archive. (Default: `/nojournal/bin/v2x-data`)
  
3. Serverside deployment only:
  - `["PullFromIntersections"]`: a `bool` specifying whether or not to pull the data from intersections. (NOTE: `true` requires connectivity with intersections)
  - `["PushToCyverse"]`: a `bool` specifying whether or not to push the data to CyVerse. (NOTE: `true` requires internet connection)
  - `["DataTransfer"]["StartTime_PullFromIntersections"]["hour"]`: an `int` specifying the hour component of the time at which attempt to **pull the data from intersections** must begin (range: 0-23)
  - `["DataTransfer"]["StartTime_PullFromIntersections"]["minute"]`: an `int` specifying the minute component of the time at which attempt to **push the data from intersections** must begin (range: 0-59)
  - `["DataTransfer"]["StartTime_PushToCyverse"]["hour"]`: an `int` specifying the hour component of the time at which attempt to **push the data to CyVerse** must begin (range: 0-23)
  - `["DataTransfer"]["StartTime_PushToCyverse"]["minute"]`: an `int` specifying the minute component of the time at which attempt to **push the data to CyVerse** must begin (range: 0-59)
  - `["DataTransfer"]["intersection"][n]["name"]`: a `string` specifying the name of the *nth* intersection
  - `["DataTransfer"]["intersection"][n]["ip_address"]`: a `string` specifying the IPv4 address of the *nth* intersection
  - `["DataTransfer"]["intersection"][n]["username"]`: a `string` specifying the the username that is allowed SFTP transactions with the *nth* intersection's coprocessor
  - `["DataTransfer"]["intersection"][n]["password"]`: a `string` specifying the password corresponding to the username that is allowed SFTP transactions with the *nth* intersection's coprocessor
  - `["DataTransfer"]["intersection"][n]["v2x-data_location"]`: a `string` specifying the absolute location of the v2x-data directory (in the remote file system), which consists the data archive. (Default: `/nojournal/bin/v2x-data`)
  - `["DataTransfer"]["intersection"][n]["cyverse_location"]`: a `string` specifying the location of the directory on CyVerse that corresponds to the *nth* intersection.

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
  - Minimally disrupted network connectivity between intersections and the local server
3. Serverside deployment:
  - Minimally disrupted network connectivity with internet
  - Sufficient storage space proportional to number of intersections in the corridor

# Known issues/limitations
- None
