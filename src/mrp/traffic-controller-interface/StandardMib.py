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