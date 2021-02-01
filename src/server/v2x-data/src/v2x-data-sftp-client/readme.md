# Script for transfering the data from intersections to the PBOT server
The `v2x-data-sftp-client.py` is intended to run on the server to pull the data collected on intersection coprocessors using SFTP.  The responsibilities of this script are the following:
1. Try to establish an SFTP connection with the intersection. If the connection is successful:
2. Find the `v2x-data/archive` directory located on the intersection coprocessor
3. There may be multiple directories in the archive. For each directory:
4. Using SFTP, transfer all files to the local machine and place them locally in respective folders based on the data element
5. Delete the remote directory after transfering to the server

## Pre-requisites
Following prerequisites need to be fulfilled before running the script on the server:
1. Establish and verify the network connection between the server and coprocessors hosted at different intersections of interest
2. Install operating system-specific version of [Python 3+](https://www.python.org/downloads/) on the server
3. Install following dependencies (libraries):
    - [pysftp](https://pypi.org/project/pysftp/): A library that provides simple interface to SFTP
    - [apscheduler](https://apscheduler.readthedocs.io/en/stable/): A library that scheduling interface for the Python code to be executed at defined intervals

## Configuration
Following items need to be provided in the `v2x-data-sftp-client-config.json` before running the script:
1. `"local_data_directory" -> string`: In this field enter the relative path of the directory where data needs to be stored. This directory needs to contain one directory for each intersection, where each intersection directory needs to contain five subdirectories, each for the available data element. Follow the below directory structure while creating the data storage directory:
 - local_data_directory
   - intersection-1
     - spat
     - remoteBsm
     - srm
     - ssm
     - msgCount
2. `intersections -> list`: Each element of this list represents the data pertaining to one intersection. For each intersection, following information is required (all strings):
    ```
    {
        "name": $(Name of the intersection),
        "ip_address": $(IP Address of the intersection),
        "username": $(Username having SFTP access),
        "password": $(Password associated with the username),
        "v2x-data_location": $(Absolute location where v2x-data directory exists on the intersection coprocessor)
    }
   ```
3. `data_transfer_start_24h`: In this field, provide hour and minute in the 24-hour format when the data transfer must take place. For example, if it is desired to transfer the data starting midnight, both `hour` and `minute` field should have `0` as a value.

## Launching the script
Based on the operating system and desired mode of the script (foreground or background) execute appropriate commands from the ones provided below:
1. Windows or Linux (Foreground): `python3 v2x-data-sftp-client.py`
Note: If the script is started in the foreground, it can be stopped by simply closing the terminal that is running the script.
2. Windows (Background): `pythonw3 v2x-data-sftp-client.py`
Note: If the script is started in the background, it can only be stopped using the system task manager.
3. Linux (Background): `python3 v2x-data-sftp-client.py > /dev/null 2>&1 &`
Note: If the script is started in the background, it can be stopped by executing `pkill -9 -f v2x-data-sftp-client.py`



