#-----------------------------------------------------------------------------#
#    Dockerfile to build an image to run MMITSS applications natively         #
#    The image can be used to spawn containers that can simulate each         #
#         RSE that needs to be present in the simulated network               #
#-----------------------------------------------------------------------------#

FROM ubuntu:14.04

MAINTAINER Sriharsha Mucheli (sriharsha@email.arizona.edu)

RUN apt-get update
RUN apt-get upgrade -y
RUN apt-get install build-essential -y

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

# Environment variables
ENV PATH $PATH:/mmitss

# perform a sysupgrade and install some necessary packages 
RUN apt-get update && apt-get upgrade -y && apt-get install -y build-essential wget gdb ssh libperl-dev libglpk-dev

# Download, configure, build and install libnetsnmp and delete its sources
RUN wget -O - http://downloads.sourceforge.net/project/net-snmp/net-snmp/5.7.3/net-snmp-5.7.3.tar.gz | tar -xzf - -C /root/ && cd /root/net-snmp-5.7.3/ && ./configure --prefix=/usr/ --with-default-snmp-version="3" --with-sys-contact="@@no.where" --with-sys-location="Unknown" --with-logfile="/var/log/snmpd.log" --with-persistent-directory="/var/net-snmp" && make && make install && rm -rf /root/net-snmp-5.7.3

# Add the libj2735-linux library from the source tree
ADD libj2735-linux.a /usr/lib/
