#This script configures the directory structure and libraries required by the MMITSS applications.
#The intersection configuration files will be stored in /nojournal/bin/
#The log files for each simulation run will be stored in /nojournal/bin/log/

#*********************************************************************************************
#THIS SCRIPT NEEDS A SUPERUSER ACCESS TO RUN, AS IT CREATES DIRECTORIES IN THE ROOT FOLDER.  *
#*********************************************************************************************

#Maintainer: Niraj Altekar

#Request the user-name and user-group
read -p "Username: " username
read -p "User Group: " usergroup

echo "Creating required directories in the root folder."
sudo mkdir -p /nojournal/bin/log
sudo mkdir /usr/local/lib/mmitss
sleep 1s

echo "Copy the configuration files of the intersection RSE81_Campbell to /nojournal/bin/"
sudo cp -r ./docker/corridors/speedway/rse81_campbell/nojournal/bin /nojournal
sleep 1s

echo "Change the owner and group of the configuration files and provide necessary permissions (chmod 777)"
sudo chown -R $username:$usergroup /nojournal
sudo chmod -R 777 /nojournal
sleep 1s

echo "Add the shared libraries we need to run"
sudo cp ./3rdparty/net-snmp/lib/libnetsnmp.so.35.0.0 /usr/local/lib/mmitss/
sudo cp ./3rdparty/glpk/lib/libglpk.so.35.1.0 /usr/local/lib/mmitss/
sudo cp ./lib/libmmitss-common.so /usr/local/lib/mmitss/
sudo cp ./lib/mmitss.conf /etc/ld.so.conf.d/
sleep 1s

echo "Create the symbolic links for the copied libraries."
sudo ln -s /usr/local/lib/mmitss/libnetsnmp.so.35.0.0 /usr/local/lib/mmitss/libnetsnmp.so.35
sudo ln -s /usr/local/lib/mmitss/libglpk.so.35.1.0 /usr/local/lib/mmitss/libglpk.so.35
sudo ldconfig
pkill -9 sleep
