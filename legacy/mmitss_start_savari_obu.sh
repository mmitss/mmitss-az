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
# mmitss_start_savari_obu.sh                                                                     
# Created by Niraj Altekar                                                                  
# Transportation Research Institute                                                         
# Systems and Industrial Engineering                                                        
# The University of Arizona                                                                 
#                                                                                           
# This code was develop under the supervision of Professor Larry Head                       
# in the Transportation Research Institute.                                                 
#                                                                                           
# Operational Description:                                                                  
# This script launches the patches developed by Savari for match the needs of MMITSS apps.
# This script is intended to run atleast 2 minuts after the startup of the OBU.
#                                                                                                
#############################################################################################

# Start the DSRC Message Forwarder application. This application forwards the messages received over DSRC to the vehicleside processor.
# Correct ports need to defined in /etc/config/v2vi_obe.conf and /etc/config/DsrcForward.conf. Use the scripts ./savariPatchConfig/* for configuration.
dsrc_message_forward -f /etc/config/DsrcForward.conf &
sleep 1s

/usr/local/bin/IFM &
sleep 1s
