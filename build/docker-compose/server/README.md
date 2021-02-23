# Docker Compose files for server-based deployment

This directory consists of a dedicated subdirectory for each corridor, where a docker-compose file pertaining to that corridor is located. The docker-compose files in these subdirectories need pre-built `mmitss-mrp-server:<tag>` docker image, where the tag can be specified in the docker-compose file. For building docker image, thee following dockerfile is used.
```mmitss/build/dockerfiles/x86/Dockerfile.mrp-server```

Note that currently for server-based deployment, only x86 architecture processors can be used.

After pulling (or building) the required docker image, to run any of the docker-compose file, a user needs to open a terminal in the mmitss repository's root directory, and run the following command:
```
docker-compose -f build/docker-compose/simulation/<corridor name>/docker-compose.yml up
```

Above command launches a dedicated container for each intersection within the corridor and displays their console output in the foreground:

To stop the running containers created via the docker-compose, simply press `ctrl+c` in the terminal where the output of docker-compose is being shown. After stopping the running containers, to remove the macvlan network created by the docker-compose file, run the following command:
```
docker-compose -f build/docker-compose/simulation/<corridor name>/docker-compose.yml down
```

## IMPORTANT

While running MMITSS applications in the server-based deployment model, it is necessary to ensure unwavering network connection between intersection signal controller and the server running docker-compose. Also, it is recommended to set the `NTCIP Backup Time` of signal controllers to smaller values such as **5 seconds** to prevent any undesired effects caused due to network disruptions.
