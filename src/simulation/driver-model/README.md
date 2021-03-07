# MMITSS Driver Model for VISSIM Microscopic Simulation

To simulate connected vehicles in VISSIM simulation, the MMITSS driver-model allows vehicles using this driver-model to send their dynamic state information to a configured network node at every simulation timestep (10 Hz). 

## Work-Flow:
For each vehicle that uses the driver model, at each timestep, the VISSIM local coordinates are first transformed into GPS coordinates (Latitude, Longitude, Elevation). Along with the coordinates, other state information of the vehicle is then packed in a Binary Large Object (BLOB) whose structure is defined [here](./../README.md). The BLOB is then sent to the configured client using a UDP socket. The recommended client is the [Simulated-Bsm-Blob-Processor](./../simulated-bsm-blob-processor) component that provides methods for unpacking the blob.

## Console Output and Logging
The driver-model neither produces any console output nor generates any log files.

## Configuration
The configuration file named `mmitss_driver_model_config.json` needs to be placed in a directory named `mmitss_simulation`. The `mmitss_simulation` directory needs to be placed in the directory where simulation's `*.inpx` file is stored. The fields in the `mmitss_driver_model_config.json` are the following:
1. `["client_ip"]`: a `string` specifying the IPv4 address of the client that should receive the generated BLOBs.
2. `["client_port"]`: a `string` specifying the UDP port of the client that should receive the generated BLOBs.   

## Requirements

## Build Instructions

## Known Issues/Limitations
