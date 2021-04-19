#!/bin/bash
#############################################################################################
#                                                                                           
# NOTICE:  Copyright 2018 Arizona Board of Regents on behalf of University of Arizona.      
# All information, intellectual, and technical concepts contained herein is and shall       
# remain the proprietary information of Arizona Board of Regents and may be covered         
# by U.S. and Foreign Patents, and patents in process.  Dissemination of this information          
# or reproduction of this material is strictly forbidden unless prior written permission    
# is obtained from Arizona Board of Regents or University of Arizona.                       
#                                                                                           
# launch-container.sh                                                                     
# Created by Niraj Altekar                                                                  
# Transportation Research Institute                                                         
# Systems and Industrial Engineering                                                        
# The University of Arizona                                                                 
#                                                                                           
# This code was develop under the supervision of Professor Larry Head                       
# in the Transportation Research Institute.                                                 
#                                                                                           
# Operational Description:                                                                   
# This script launches the docker container after getting inputs from the user                                                                                               
#############################################################################################

read -p "Full absolute path of MMITSS configuration directory (with no trailing /): " -e config_path
read -p "Name of container image on the Dockerhub : " container_image
read -p "Name of container: " container_name
read -p "Specify timezone string: " timezone


docker run --privileged -d --restart always -v $config_path:/nojournal -e TZ=$timezone --network host --name $container_name $container_image > /dev/null 2>&1 &
