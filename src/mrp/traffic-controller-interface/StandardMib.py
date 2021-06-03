"""
***************************************************************************************

 © 2019 Arizona Board of Regents on behalf of the University of Arizona with rights
       granted for USDOT OSADP distribution with the Apache 2.0 open source license.

***************************************************************************************

StandardMib.py
Created by: Niraj Vasant Altekar
University of Arizona   
College of Engineering

This code was developed under the supervision of Professor Larry Head
in the Systems and Industrial Engineering Department.

***************************************************************************************

Description:
------------
This module stores the OIDs of NTCIP objects from the NTCIP 1202 V2 standard. 
The OIDs stored in this module are independent of the vendor of the traffic
signal controller.

***************************************************************************************
"""
# Phase Parameters:
# Need to add the last digit denoting the number of phase. 
# For example, to get the ring of phase 2, the corresponding OID would be "1.3.6.1.4.1.1206.4.2.1.1.2.1.22.2"

PHASE_PARAMETERS_PHASE_NUMBER   =   "1.3.6.1.4.1.1206.4.2.1.1.2.1.1."
PHASE_PARAMETERS_PEDWALK        =   "1.3.6.1.4.1.1206.4.2.1.1.2.1.2."
PHASE_PARAMETERS_PEDCLEAR       =   "1.3.6.1.4.1.1206.4.2.1.1.2.1.3."
PHASE_PARAMETERS_MIN_GRN        =   "1.3.6.1.4.1.1206.4.2.1.1.2.1.4."   
PHASE_PARAMETERS_PASSAGE        =   "1.3.6.1.4.1.1206.4.2.1.1.2.1.5."
PHASE_PARAMETERS_MAX_GRN        =   "1.3.6.1.4.1.1206.4.2.1.1.2.1.6."
PHASE_PARAMETERS_YELLOW_CHANGE  =   "1.3.6.1.4.1.1206.4.2.1.1.2.1.8."
PHASE_PARAMETERS_RED_CLR        =   "1.3.6.1.4.1.1206.4.2.1.1.2.1.9."

PHASE_PARAMETERS_RING           =   "1.3.6.1.4.1.1206.4.2.1.1.2.1.22."

# Phase Group Statuses, which are not available in the SPAT blob: 

PHASE_GROUP_STATUS_NEXT         =   "1.3.6.1.4.1.1206.4.2.1.1.4.1.11.1"

# Phase Control Objects:
PHASE_CONTROL_VEH_OMIT          =   "1.3.6.1.4.1.1206.4.2.1.1.5.1.2.1"
PHASE_CONTROL_PED_OMIT          =   "1.3.6.1.4.1.1206.4.2.1.1.5.1.3.1"
PHASE_CONTROL_HOLD              =   "1.3.6.1.4.1.1206.4.2.1.1.5.1.4.1"
PHASE_CONTROL_FORCEOFF          =   "1.3.6.1.4.1.1206.4.2.1.1.5.1.5.1"
PHASE_CONTROL_VEHCALL           =   "1.3.6.1.4.1.1206.4.2.1.1.5.1.6.1"
PHASE_CONTROL_PEDCALL           =   "1.3.6.1.4.1.1206.4.2.1.1.5.1.7.1"    

# Phase calls:
VEHICLE_CALLS                   =   "1.3.6.1.4.1.1206.4.2.1.1.4.1.8.1"
PEDESTRIAN_CALLS                =   "1.3.6.1.4.1.1206.4.2.1.1.4.1.9.1"

# NTCIP Backup Time Parameter
NTCIP_UNIT_BACKUP_TIME          =   "1.3.6.1.4.1.1206.4.2.1.3.3.1"

# Special Function
TOTAL_SPECIAL_FUNCTIONS         =   "1.3.6.1.4.1.1206.4.2.1.3.13.1"
SPECIAL_FUNCTION                =   "1.3.6.1.4.1.1206.4.2.1.3.14.1.3"