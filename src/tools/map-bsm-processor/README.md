# map_bsm_processor
This repository contains source code for processing bsm data when the map-payload for the intersection is available. It contains the map-engine library developed by California PATH, and a data processing wrapper developed by the UArizona Transportation Research Institute.

## Prerequisites
The base map-engine library is already prebuilt. For Building and running the data processing wrapper, following prerequisites must be atisfied on the host machine:
1. Processor Architecture: 64-bit (x86)
2. Operating System: Ubuntu 18.04 LTS (with admin access)
3. Required package(s): build-essential (which can be installed by running `sudo apt-get install build-essential`

## Initializing Map-Engine library
As mentioned earlier, the base map-engine library is already pre-built and stored in the repository in `.so` format. However, this and other associated libraries need to be copied to `/usr/lib` folder of the host machine. To do so, execute the following steps:
1. Open a terminal and go to the config directory of the repository.
2. Execute `./initialize-map-engine.sh` command to run the script that copies pre-built libraries to the required location. Note that commands in this script require `sudo` access.
## Building Data Processing Wrapper: map-bsm-processor
After prerequisites are satisfied and map-engine libraries are copied to `/usr/lib` folder, execute the following steps to build the data processing wrapper. 
1. Open a terminal and go to the `src` folder of the repository
2. Execute `make clean;make linux` command to build the wrapper.
3. After building the wrapper a binary named `MapBsmProcessor` will be created in the `src` folder.
## Running map-bsm-processor application
The MapBsmProcesser uses map-engine libraries to process the BSM data stored in a csv file. 
### Data
The MapBsmProcesser requires following prerequisites for the stored data:
1. The data is stored in a csv formatted file containing the keyword `remoteBsm`. For example the name of the file that stores the raw data could be `remoteBsmLog.csv`
2. The CSV file must have columns in the following sequence:
`log_timestamp_verbose`, `log_timestamp_posix`, `timestamp_verbose`, `timestamp_posix`, `temporaryId`, `secMark`, `latitude`, `longitude`, `elevation`, `speed`, `heading`, `type`, `length`, `width`
3. Note that columns `log_timestamp_verbose`, `log_timestamp_posix`, `timestamp_verbose`, `timestamp_posix`, `secMark`, `type`, `length`, and `width` may contain dummy data. It will not affect the performace of the processing.
4. An example of such raw data file is available in the `data` folder of the repository.
### Running the application
To process the raw data based on the map, execute following steps:
1. Open a terminal and go to the `src` folder of the repository
2. Execute `./MapBsmProcessor <path to remoteBsmLog.csv file>`
3. Upon execution of the above command, a new file will be generated in the same location where `remoteBsmLog.csv` file was stored with the name `processedRemoteBsmLog.csv`. This file will contain the additional information that is processed by the map-engine.
