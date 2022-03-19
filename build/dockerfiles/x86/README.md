# Dockerfiles for x86 Processors

This directory contains Dockerfiles that can be used to create container images to run under x86 architecture. To build a docker image using any of these Dockerfiles, the user needs to open a terminal in MMITSS repository's root directory and execute the following command:
```
docker build -t <desired name of the image>:<version tag> -f build/dockerfiles/x86/<name of the Dockerfile> .
```

Following Dockerfiles are available:
1. `Dockerfile.base` is used to create the base image for MMITSS containers. This image is based on ubuntu 20.04 (previous image was based on ubuntu 18.04) container image. Key responsibilities covered by the Dockerfile include installing packages required for maintaining child containers and copying static and dynamic libraries to appropriate locations in the container file system.
2. `Dockerfile.mrp-server` is used to create the container image for running MMITSS applications in server-based deployment model.
3. `Dockerfile.mrp-simulation` is used to create the container image for running MMITSS applications in the simulation environment.
4. `Dockerfile.simulation-tools` builds the image that contains the applications required for facilitating the MMITSS simulation, i.e., MessageDistributor, SimulatedBsmBlobProcessor, and PriorityRequestGeneratorServer.
