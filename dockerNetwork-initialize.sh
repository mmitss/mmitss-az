#############################################################################################
#                                                                                           #
# NOTICE:  Copyright 2018 Arizona Board of Regents on behalf of University of Arizona.      #
# All information, intellectual, and technical concepts contained herein is and shall       #
# remain the proprietary information of Arizona Board of Regents and may be covered         #
# by U.S. and Foreign Patents, and patents in process.  Dissemination of this information   #       
# or reproduction of this material is strictly forbidden unless prior written permission    #
# is obtained from Arizona Board of Regents or University of Arizona.                       #
#                                                                                           #
# dockerNetwork_initialize.sh                                                               #
# Created by Niraj Altekar                                                                  #
# Transportation Research Institute                                                         #
# Systems and Industrial Engineering                                                        #
# The University of Arizona                                                                 #
#                                                                                           #
# This code was develop under the supervision of Professor Larry Head                       #
# in the Transportation Research Institute.                                                 #
#                                                                                           #
# Revision History:                                                                         #
# Rev00: Initial Release.                                                                   #
# Once docker is installed, this script creates a macvlan network bridge that facilitates   #
# communication between the containers and relative outside world.                          #
#                                                                                           #
#############################################################################################


echo "Running ifconfig to list the available network interfaces"
sleep 1s
ifconfig

echo "Enter the name of ethernet port:"
read ethernetPortName
echo "Enter the subnet of ethernet port. Example: 10.254.56.0"
read ethernetSubnet
ethernetSubnetMask="$ethernetSubnet/24" #appends the default mask of 255.255.255.0 on the desired subnet.
echo "Enter the desired gateway. Example for given subnet:10.254.56.1"
read ethernetGateway
echo "Creating a macvlan network..."
sleep 1s

echo "Name of ethernet port is: "$ethernetPortName
sleep 1s
echo "Subnet is: "$ethernetSubnetMask
sleep 1s
echo "Gateway is: "$ethernetGateway
sleep 1s

sudo docker network create -d macvlan --subnet=$ethernetSubnetMask --gateway=$ethernetGateway -o parent=$ethernetPortName macvlan_1
echo "Macvlan network is now established successfully!"
