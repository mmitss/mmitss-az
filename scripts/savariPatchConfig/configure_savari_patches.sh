#!/bin/bash

read -p "Enter the IP address of the RaspberryPi associated with this OBU: " raspiIp
sed -i 's/IPADDRESS/'${raspiIp}'/g' DsrcForward.conf v2vi_obe.conf
cp DsrcForward.conf /etc/config/
cp v2vi_obe.conf /etc/config
sed -i 's/'${raspiIp}'/IPADDRESS/g' DsrcForward.conf v2vi_obe.conf
