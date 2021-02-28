# Docker Compose files for simulation deployment

This directory consists of a dedicated subdirectory for each corridor, where a docker-compose file pertaining to that corridor is located. The simulation docker-compose files do not need any prebuilt docker image, as it builds the required docker image on the go. For building docker images, following dockerfiles are used depending upon the processor architecture:
- ARM: mmitss/build/dockerfiles/arm/Dockerfile.mrp-simulation and mmitss/build/dockerfiles/arm/Dockerfile.simulation-tools
- x86: mmitss/build/dockerfiles/x86/Dockerfile.mrp-simulation and mmitss/build/dockerfiles/x86/Dockerfile.simulation-tools

Before running any corridor-specifc docker-compose file, it is necessary to build all MMITSS applications listed in the corresponding Dockerfile (one from above two options). To do so, run `build-x86.sh` or `build-arm.sh` script from the `mmitss/scripts` folder depending upon the processor architecture.

After required applications are built, to run any of the docker-compose file, a user needs to open a terminal in the mmitss repository's root directory, and run the following command:
```
docker-compose -f build/docker-compose/simulation/<corridor name>/docker-compose.yml up --build
```

If the docker image is already built, `--build` flag can be skipped. That is, in such case the command to run the docker-compose file would be:
```
docker-compose -f build/docker-compose/simulation/<corridor name>/docker-compose.yml up
```

Above commands launch following types of containers, and display their console output in the foreground:
- A dedicated container for each intersection within the corridor
- A container that hosts simulation specific applications (i.e., MessageDistributor, SimulatedBsmBlobProcessor, and PriorityRequestGeneratorServer)

To stop the running containers created via the docker-compose, simply press `ctrl+c` in the terminal where the output of docker-compose is being shown. After stopping the running containers, to remove the macvlan network created by the docker-compose file, run the following command:
```
docker-compose -f build/docker-compose/simulation/<corridor name>/docker-compose.yml down
```


