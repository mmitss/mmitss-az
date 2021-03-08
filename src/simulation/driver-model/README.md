# MMITSS Driver Model for VISSIM Microscopic Simulation

To simulate connected vehicles in VISSIM simulation, the MMITSS driver-model allows vehicles using this driver-model to send their dynamic state information to a configured network node at every simulation timestep (10 Hz). 

## Work-Flow:
For each vehicle that uses the driver model, at each timestep, the VISSIM local coordinates are first transformed into GPS coordinates (Latitude, Longitude, Elevation). Along with the coordinates, other state information of the vehicle is then packed in a Binary Large Object (BLOB) whose structure is defined [here](./../README.md). The BLOB is then sent to the configured client using a UDP socket. The recommended client is the [Simulated-Bsm-Blob-Processor](./../simulated-bsm-blob-processor) component that provides methods for unpacking the blob.

## Console Output and Logging
The driver-model neither produces any console output nor generates any log files.

## Configuration
Following fields in the `mmitss_driver_model_config.json` need to be configured prior to the simulation run:
1. `["client_ip"]`: a `string` specifying the IPv4 address of the client that should receive the generated BLOBs.
2. `["client_port"]`: a `string` specifying the UDP port of the client that should receive the generated BLOBs.   
3. `["vissim_origin_position"]`: GPS coordinates of the origin (0,0) point of the simulation model.
    - `["vissim_origin_position"]["latitude"]["Degree"]`, `["vissim_origin_position"]["latitude"]["Minute"]`, and `["vissim_origin_position"]["latitude"]["Second"]`: The latitude of the origin of the simulation, in Degree, Minute, Second notation.
    - `["vissim_origin_position"]["longitude"]["Degree"]`, `["vissim_origin_position"]["longitude"]["Minute"]`, and `["vissim_origin_position"]["longitude"]["Second"]`: The longitude of the origin of the simulation, in Degree, Minute, Second notation.
    - `["vissim_origin_position"]["elevation_Meter"]`: Elevation of the origin of the simulation, in meter.

## Requirements
1. The configuration file: `mmitss_driver_model_config.json` needs to be placed in a directory: `mmitss_simulation`. The `mmitss_simulation` directory needs to be placed in the sae directory where the `*.inpx` file of the simulation is stored. 
2. Ensure that the simulation uses only **single** CPU core during the simulation run.

## Build Instructions
The driver model can be built using [Microsoft Visual Studio](https://visualstudio.microsoft.com/free-developer-offers/). Execute the following steps to build the driver-model DLL from scratch:
1. Open `DriverModel.vcxproj` file in the Microsoft Visual Studio.
2. From the project, open DriverModel.cpp
3. Based on the requirement use the appropriate Vehicle Type identifier on line 45.
4. Under `Solution Configurations` dropdown, select **Release**
5. Under `Solution Platform` dropdown, select **x64**
6. Press `ctrl+shift+B` to build the driver-model.
7. Once step 6 is completed, a file: `ConnectedVehicle.dll` will be generated in the `<directory of DriverModel.vcxproj>\x64\Release\` directory.
8. The generated DLL may be renamed to hint the embedded vehicle type. Vehicles using this DLL will send their state information to the configured client at every simulation timestep.

## Known Issues/Limitations
1. The driver-model is by design a DLL intended to be built and used on Microsoft Windows platform only.
2. The structure of the BLOB needs to be known by the client that receives the BLOBs. 
