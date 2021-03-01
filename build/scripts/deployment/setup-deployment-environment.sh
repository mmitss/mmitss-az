#############################################################################################
#                                                                                           #
# NOTICE:  Copyright 2018 Arizona Board of Regents on behalf of University of Arizona.      #
# All information, intellectual, and technical concepts contained herein is and shall       #
# remain the proprietary information of Arizona Board of Regents and may be covered         #
# by U.S. and Foreign Patents, and patents in process.  Dissemination of this information   #       
# or reproduction of this material is strictly forbidden unless prior written permission    #
# is obtained from Arizona Board of Regents or University of Arizona.                       #
#                                                                                           #
# setup-deployment-environment.sh                                                           #
# Created by Niraj Altekar                                                                  #
# Transportation Research Institute                                                         #
# Systems and Industrial Engineering                                                        #
# The University of Arizona                                                                 #
#                                                                                           #
# This code was develop under the supervision of Professor Larry Head                       #
# in the Transportation Research Institute.                                                 #
#                                                                                           #
#############################################################################################



read -p "Architecture - x86 or arm: " arch
read -p "Name of network adapter to be used by MMITSS: " mmitss_network_adapter

echo "Adding MMITSS_ROOT to ~/.bashrc"
echo "export MMITSS_ROOT=$(pwd)/../../.." >> ~/.bashrc

echo "Adding MMITSS_NETWORK_ADAPTER to ~/.bashrc"
echo "export MMITSS_NETWORK_ADAPTER=$mmitss_network_adapter" >> ~/.bashrc

echo "Adding PROCESSOR to ~/.bashrc"
echo "export PROCESSOR=$arch" >> ~/.bashrc

sleep 2
echo "Added required environment variables in ~/.bashrc file."
echo "To allow for changes to take effect, either close this terminal or execute the command: source ~/.bashrc"




