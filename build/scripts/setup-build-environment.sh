#############################################################################################
#                                                                                           #
# NOTICE:  Copyright 2018 Arizona Board of Regents on behalf of University of Arizona.      #
# All information, intellectual, and technical concepts contained herein is and shall       #
# remain the proprietary information of Arizona Board of Regents and may be covered         #
# by U.S. and Foreign Patents, and patents in process.  Dissemination of this information   #       
# or reproduction of this material is strictly forbidden unless prior written permission    #
# is obtained from Arizona Board of Regents or University of Arizona.                       #
#                                                                                           #
# setup-build-environment.sh                                                                #
# Created by Niraj Altekar                                                                  #
# Transportation Research Institute                                                         #
# Systems and Industrial Engineering                                                        #
# The University of Arizona                                                                 #
#                                                                                           #
# This code was develop under the supervision of Professor Larry Head                       #
# in the Transportation Research Institute.                                                 #
#                                                                                           #
# Revision History:                                                                         #
# This script configures the directory structure and libraries required by the MMITSS apps. #
# The intersection configuration files will be stored in /nojournal/bin/                    #
# The log files for each simulation run will be stored in /nojournal/bin/log/               #
#                                                                                           #
#############################################################################################



#*********************************************************************************************
#THIS SCRIPT NEEDS A SUPERUSER ACCESS TO RUN, AS IT CREATES DIRECTORIES IN THE ROOT FOLDER.  *
#*********************************************************************************************

#Request the user-name, user-group, and architecture
read -p "Username: " username
read -p "User Group: " usergroup
read -p "Architecture - x86 or arm: " arch
read -p "Name of network adapter to be used by MMITSS: " mmitss_network_adapter
read -p "Copy sample configuration file to /nojournal/bin? (y or n): " copy_sample 

echo "Adding MMITSS_ROOT to ~/.bashrc"
echo "export MMITSS_ROOT=$(pwd)/../../.." >> ~/.bashrc

echo "Adding MMITSS_NETWORK_ADAPTER to ~/.bashrc"
echo "export MMITSS_NETWORK_ADAPTER=$mmitss_network_adapter" >> ~/.bashrc

echo "Adding PROCESSOR to ~/.bashrc"
echo "export PROCESSOR=$arch" >> ~/.bashrc

echo "Adding environment variable required for SNMP sessions to ~/.bashrc"
echo "export MIBS=ALL" >> ~/.bashrc

echo "Creating required directories in the root folder."
sudo rm -r /nojournal/
sudo rm -r /usr/local/lib/mmitss
sudo mkdir -p /nojournal/bin/log
sudo mkdir /usr/local/lib/mmitss
sleep 1s

if [ "$copy_sample" = "y" ]; then
echo "Copy the configuration files of the intersection speedway-mountain to /nojournal/bin/"
sudo cp -r ../../config/speedway-sample/simulation/speedway-mountain/nojournal/bin /nojournal
sleep 1s
echo "Change the owner and group of the configuration files and provide necessary permissions (chmod 777)"
sudo chown -R $username:$usergroup /nojournal
sudo chmod -R 777 /nojournal
sleep 1s
fi

echo "Add the shared libraries we need to run"

if [ "$arch" = "x86" ]; then
sudo cp ../../3rdparty/net-snmp/lib/x86/libnetsnmp.so.35.0.0 /usr/local/lib/mmitss/
sudo cp ../../3rdparty/glpk/lib/x86/libglpk.so.35.1.0 /usr/local/lib/mmitss/
sudo cp ../../lib/x86/libmmitss-common.so /usr/local/lib/mmitss/
sudo cp ../../3rdparty/mapengine/lib/x86/liblocAware.so.1.0 /usr/local/lib/mmitss/
sudo cp ../../3rdparty/asn1j2735/lib/x86/libasn.so.1.0 /usr/local/lib/mmitss/
sudo cp ../../3rdparty/asn1j2735/lib/x86/libdsrc.so.1.0 /usr/local/lib/mmitss/
sudo cp ../../lib/mmitss.conf /etc/ld.so.conf.d/
echo "Create the symbolic links for the copied libraries."
sudo ln -s /usr/local/lib/mmitss/libnetsnmp.so.35.0.0 /usr/local/lib/mmitss/libnetsnmp.so.35
sudo ln -s /usr/local/lib/mmitss/libglpk.so.35.1.0 /usr/local/lib/mmitss/libglpk.so.35
sudo ln -s /usr/local/lib/mmitss/liblocAware.so.1.0 /usr/local/lib/mmitss/liblocAware.so
sudo ln -s /usr/local/lib/mmitss/libasn.so.1.0 /usr/local/lib/mmitss/libasn.so
sudo ln -s /usr/local/lib/mmitss/libdsrc.so.1.0 /usr/local/lib/mmitss/libdsrc.so
fi

if [ "$arch" = "arm" ]; then
sudo cp ../../3rdparty/net-snmp/lib/arm/libnetsnmp.so.35.0.0 /usr/local/lib/mmitss/
sudo cp ../../3rdparty/glpk/lib/arm/libglpk.so.40.3.0 /usr/local/lib/mmitss/
sudo cp ../../lib/arm/libmmitss-common.so /usr/local/lib/mmitss/
sudo cp ../../3rdparty/mapengine/lib/arm/liblocAware.so.1.0 /usr/local/lib/mmitss/
sudo cp ../../3rdparty/asn1j2735/lib/arm/libasn.so.1.0 /usr/local/lib/mmitss/
sudo cp ../../3rdparty/asn1j2735/lib/arm/libdsrc.so.1.0 /usr/local/lib/mmitss/
sudo cp ../../lib/mmitss.conf /etc/ld.so.conf.d/
sudo cp ../../3rdparty/openssl/* /usr/local/lib


echo "Create the symbolic links for the copied libraries."
sudo ln -s /usr/local/lib/mmitss/libnetsnmp.so.35.0.0 /usr/local/lib/mmitss/libnetsnmp.so.35
sudo ln -s /usr/local/lib/mmitss/libglpk.so.40.3.0 /usr/local/lib/mmitss/libglpk.so.40
sudo ln -s /usr/local/lib/mmitss/liblocAware.so.1.0 /usr/local/lib/mmitss/liblocAware.so
sudo ln -s /usr/local/lib/mmitss/libasn.so.1.0 /usr/local/lib/mmitss/libasn.so
sudo ln -s /usr/local/lib/mmitss/libdsrc.so.1.0 /usr/local/lib/mmitss/libdsrc.so
fi

sleep 1s

sudo ldconfig

sleep 2s
echo "Added required environment variables in ~/.bashrc file."
echo "To allow for changes to take effect, either close this terminal or execute the command: source ~/.bashrc"
pkill -9 sleep #End
