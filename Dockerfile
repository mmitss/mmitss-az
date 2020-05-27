#-----------------------------------------------------------------------------#
#    Dockerfile to build an image to run MMITSS applications natively         #
#    The image can be used to spawn containers that can simulate each         #
#         RSE that needs to be present in the simulated network               #
#-----------------------------------------------------------------------------#
FROM ubuntu:18.04

MAINTAINER D Cunningham (pearson10m@gmail.com)

#RUN apt-get update
#RUN apt-get upgrade -y
#RUN apt-get install build-essential -y

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

# Environment variables
ENV PATH $PATH:/mmitss

# iputils-ping iproute2 gdb ddd

# perform a sysupgrade and install some necessary packages 
RUN apt-get update && apt-get upgrade -y && apt-get install -y build-essential wget gdb ddd ssh libperl-dev libglpk-dev libssl-dev

# Download, configure, build and install libnetsnmp and delete its sources
RUN wget -O - https://sourceforge.net/projects/net-snmp/files/net-snmp/5.8/net-snmp-5.8.tar.gz/download | tar -xzf - -C /root/ && cd /root/net-snmp-5.8/ && ./configure --prefix=/usr/ --with-default-snmp-version="3" --with-sys-contact="@@no.where" --with-sys-location="Unknown" --with-logfile="/var/log/snmpd.log" --with-persistent-directory="/var/net-snmp" && make && make install && rm -rf /root/net-snmp-5.8

# Download and install pip managing python libraries
RUN sudo apt-get install python3-pip

# Download and install required python libraries
RUN pip3 install haversine
RUN pip3 install apscheduler
RUN pip3 install easysnmp
RUN pip3 install sh

# Add the libj2735-linux library from the source tree
#ADD libj2735-linux.a /usr/lib/

#Add the shared libraries we need to run
#ADD ./3rdparty/net-snmp/lib/libnetsnmp.so.30.0.3 /usr/local/lib/mmitss/
ADD ./3rdparty/glpk/lib/libglpk.so.35.1.0 /usr/local/lib//mmitss/
ADD ./lib/libmmitss-common.so /usr/local/lib/mmitss/
ADD ./lib/mmitss.conf /etc/ld.so.conf.d
 
#RUN ln -s /usr/local/lib/mmitss/libnetsnmp.so.30.0.3 /usr/local/lib/mmitss/libnetsnmp.so.30
RUN ln -s /usr/local/lib/mmitss/libglpk.so.35.1.0 /usr/local/lib/mmitss/libglpk.so.35
RUN ldconfig
