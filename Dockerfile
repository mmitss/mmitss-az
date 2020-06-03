#-----------------------------------------------------------------------------#
#    Dockerfile to build an image to run MMITSS applications natively         #
#    The image can be used to spawn containers that can simulate each         #
#         RSU that needs to be present in the simulated network               #
#-----------------------------------------------------------------------------#
FROM ubuntu:18.04

MAINTAINER D Cunningham (pearson10m@gmail.com)

# iputils-ping iproute2 gdb ddd

# perform a sysupgrade and install some necessary packages 
RUN apt-get update && apt-get upgrade -y && apt-get install -y build-essential wget gdb ddd ssh libperl-dev libglpk-dev libssl-dev

# Download and install pip managing python libraries
RUN apt-get install python3-pip

# Download and install required python libraries
RUN pip3 install haversine && pip3 install apscheduler && pip3 install easysnmp && pip3 install sh

# Add the shared libraries we need to run
COPY ./3rdparty/net-snmp/lib/x86/libnetsnmp.so.35.0.0 /usr/local/lib/mmitss/
COPY ./3rdparty/glpk/lib/x86/libglpk.so.35.1.0 /usr/local/lib/mmitss/
COPY ./lib/x86/libmmitss-common.so /usr/local/lib/mmitss/
COPY ./3rdparty/mapengine/lib/x86/liblocAware.so.1.0 /usr/local/lib/mmitss/
COPY ./3rdparty/asn1j2735/lib/x86/libasn.so.1.0 /usr/local/lib/mmitss/
COPY ./3rdparty/asn1j2735/lib/x86/libdsrc.so.1.0 /usr/local/lib/mmitss/
COPY ./lib/mmitss.conf /etc/ld.so.conf.d/

# Create the symbolic links for the copied libraries."
RUN ln -s /usr/local/lib/mmitss/libnetsnmp.so.35.0.0 /usr/local/lib/mmitss/libnetsnmp.so.35 && ln -s /usr/local/lib/mmitss/libglpk.so.35.1.0 /usr/local/lib/mmitss/libglpk.so.35 //
&& ln -s /usr/local/lib/mmitss/liblocAware.so.1.0 /usr/local/lib/mmitss/liblocAware.so && ln -s /usr/local/lib/mmitss/libasn.so.1.0 /usr/local/lib/mmitss/libasn.so //
&& ln -s /usr/local/lib/mmitss/libdsrc.so.1.0 /usr/local/lib/mmitss/libdsrc.so && ldconfig

# Environment variables
ENV PATH $PATH:/mmitss

# Expose ports to communicate with outside world
# For SRM
EXPOSE 4444/udp

# For BSM
EXPOSE 3333/udp

# For SPaT/MAP
EXPOSE 15030/udp

# For ART
EXPOSE 15040/udp

# For VISSIM
EXPOSE 30000/udp
EXPOSE 20000/udp

# For Controller
EXPOSE 501/udp

# MMITSS Phase-3 Applications

# Transceivers
EXPOSE 10001/udp
EXPOSE 10002/udp
EXPOSE 10003/udp
EXPOSE 10004/udp
EXPOSE 10005/udp
EXPOSE 10006/udp

# Applications
EXPOSE 20001/udp
EXPOSE 20002/udp
EXPOSE 20003/udp
EXPOSE 20004/udp
EXPOSE 20005/udp
EXPOSE 20006/udp
EXPOSE 20007/udp
EXPOSE 20008/udp
EXPOSE 20009/udp
EXPOSE 20010/udp
EXPOSE 20011/udp

# Data Collectors
EXPOSE 30001/udp

# Additional ports
EXPOSE 50003/udp

# Requirement specific ports
EXPOSE 6053/udp
EXPOSE 1516/udp
