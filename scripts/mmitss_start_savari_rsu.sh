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
# lmmitss-initialize.sh                                                                     
# Created by Niraj Altekar                                                                  
# Transportation Research Institute                                                         
# Systems and Industrial Engineering                                                        
# The University of Arizona                                                                 
#                                                                                           
# This code was develop under the supervision of Professor Larry Head                       
# in the Transportation Research Institute.                                                 
#                                                                                           
# Revision History:                                                                         
# Rev00: Initial Release.                                                                   
# This script configures the directory structure and libraries required by the MMITSS apps. 
# The intersection configuration files will be stored in /nojournal/bin/                    
# The log files for each simulation run will be stored in /nojournal/bin/log/               
#                                                                                           
#############################################################################################
halt &>/dev/null

sleep 1

cp ./immediate_forward_ssm /usr/local/bin/savari/

cp ./immediate_forward /usr/local/bin/savari/

cp ./DsrcProxy_SSM.conf /etc/config/

run &>/dev/null

sleep 2

/usr/local/bin/savari/immediate_forward_ssm &

pgrep -f "immediate_forward_ssm" &>/dev/null
if [ $? -ne 0 ]; then
    echo "Failed to launch immediate_forward_ssm"
fi

killall ipv6_provider &>/dev/null
