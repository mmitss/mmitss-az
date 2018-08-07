#!/bin/bash
## This script is the starting point for all the MMITSS applications inside the Docker Container

run_in_bg () {
	$@ &> /nojournal/bin/log/${1}.log &
	sleep 1s
}

# ifconfig > /nojournal/bin/log/my_ip.txt

############## Peer Priority applications (0616)
# Make sure using correct argument for each applicaiton (B.H)
  run_in_bg MMITSS_MRP_PriorityRequestServer_ASN -c 2
  run_in_bg MMITSS_MRP_MAP_SPAT_Broadcast_ASN 127.0.0.1 127.0.0.1 0 
  run_in_bg MMITSS_OBE_MAP_SPAT_Receiver_ASN
       #run_in_bg MMITSS_OBE_PriorityRequestGenerator_ASN -c 2
  run_in_bg MMITSS_OBE_PriorityRequestGenerator_ASN -c 2 -l 3	
  run_in_bg MMITSS_MRP_Priority_Solver   
  run_in_bg MMITSS_MRP_TrafficControllerInterface -c 3         
       # Interface : simulation-coordination : -c 2, simulation-Non-coordination : -c 3  
       #run_in_bg MMITSS_MRP_TrafficControllerInterface_forceoffTest  
  run_in_bg Long_Term_Planning
  run_in_bg MMITSS_MRP_EquippedVehicleTrajectoryAware_ASN 20000 1 0

# Run the applications which are required
# run_in_bg MMITSS_MRP_MAP_SPAT_Broadcast_ASN 127.0.0.1 127.0.0.1 1

#Application for adaptive coordination (The order is important !!! Trajectory awareness is the first)
#  run_in_bg MMITSS_MRP_EquippedVehicleTrajectoryAware_ASN 20000 1 0
#  run_in_bg coordinator
  # penetration rate : 50% => 20000 0.5 2 0
#  run_in_bg MMITSS_rsu_Signal_Control_visual 20000 1.0 2 0
#  run_in_bg MMITSS_MRP_TrafficControllerInterface
#  run_in_bg Analysis_coordination

#Application for Time spent in the system measure (Actuated coordination Vs adaptive Or Vs free operation)
 # run_in_bg MMITSS_MRP_EquippedVehicleTrajectoryAware_ASN 20000 1 0
 # run_in_bg Analysis_coordination
 
#Data for Miao (vehicle ID identification)
 # run_in_bg MMITSS_MRP_EquippedVehicleTrajectoryAware_ASN 20000 1 0
 # run_in_bg string_data
 

# Application for PCD
#  run_in_bg MMITSS_MRP_EquippedVehicleTrajectoryAware_ASN 20000 1 0
#  run_in_bg PCD 

#  run_in_bg string_data

# run_in_bg Analysis_coordination
# run_in_bg coordinator
# run_in_bg MMITSS_rsu_Signal_Control_visual 20000 0.50 2 0
#  run_in_bg MMITSS_MRP_TrafficControllerInterface

# run_in_bg offsetrefiner
# run_in_bg MMITSS_rsu_Signal_Control_visual 20000 1.0 2
# run_in_bg MMITSS_MRP_EquippedVehicleTrajectoryAware_ASN_vissimtime 20000 1 0
# run_in_bg MMITSS_MRP_TrafficControllerInterface_vissimtime 20000
# run_in_bg MMITSS_rsu_Signal_Control_visual_coord_TSP 20000 1.0 0 20000
# run_in_bg MMITSS_rsu_PerformanceObserver_TT 20000
# run_in_bg mehdi_mprsolver_ack
# run_in_bg MMITSS_MRP_PriorityRequestServer_ackasn
# run_in_bg MMITSS_OBE_MAP_SPAT_Receiver_ASN
# run_in_bg MMITSS_OBE_PriorityRequestGenerator_ackasn -c 1
# run_in_bg NewModel


#Applications to run Integration of Signal Priority (w/w.o. cooridnation) and Adaptive Control
# run_in_bg MMITSS_MRP_Priority_Solver -c 2
# run_in_bg MMITSS_MRP_TrafficControllerInterface_vissimtime 20000     
#   run_in_bg MMITSS_MRP_EquippedVehicleTrajectoryAware_ASN_vissimtime 20000 1 0
#   run_in_bg MMITSS_rsu_Signal_Control_visual_coord_TSP 20000 1.0 0 20000
# run_in_bg MMITSS_MRP_MAP_SPAT_Broadcast_ASN 127.0.0.1 127.0.0.1 0
#  run_in_bg MMITSS_OBE_MAP_SPAT_Receiver_ASN                     
# run_in_bg MMITSS_MRP_PriorityRequestServer_sim_final -c 2 -o 0
#  run_in_bg MMITSS_OBE_PriorityRequestGenerator_sim -c 1           
###################################################################################
#Applications to run Integration of Signal Priority with Actuation
# (B.H) : To test CPCV MMITSS_OBE_MAP_SPAT_Receiver_ASN and MMITSS_OBE_PriorityRequestGenerator_sim -c 1 were used
#   run_in_bg MMITSS_MRP_MAP_SPAT_Broadcast_ASN 127.0.0.1 127.0.0.1 0 
#    run_in_bg MMITSS_OBE_MAP_SPAT_Receiver_ASN
#  run_in_bg mprSolver_1 -s 1
#    run_in_bg MMITSS_OBE_PriorityRequestGenerator_sim -c 1
#  run_in_bg MMITSS_MRP_TrafficControllerInterface     
#  run_in_bg MMITSS_MRP_PriorityRequestServer_sim_final 
#######################################################################

############## Priority applications for senior desgin team (Signal Priority)
# It was tested from Mehdi (2016. Jan)
#  run_in_bg MMITSS_MRP_MAP_SPAT_Broadcast_ASN 127.0.0.1 127.0.0.1 0 
#  run_in_bg MMITSS_OBE_MAP_SPAT_Receiver_ASN
#  run_in_bg MMITSS_OBE_PriorityRequestGenerator_sim -c 1
#  run_in_bg MMITSS_MRP_Priority_Solver
#  run_in_bg MMITSS_MRP_TrafficControllerInterface     
#  run_in_bg MMITSS_MRP_PriorityRequestServer_sim_final
#################################################################################






#Applications to run Integration of Signal Priority with Acutation and Coordination
# run_in_bg MMITSS_MRP_MAP_SPAT_Broadcast_ASN 127.0.0.1 127.0.0.1 0
# run_in_bg MMITSS_OBE_MAP_SPAT_Receiver_ASN                     
# run_in_bg MMITSS_MRP_PriorityRequestServer_sim 
# run_in_bg mprSolver                                        
# run_in_bg MMITSS_OBE_PriorityRequestGenerator_sim -c 1           
#   run_in_bg MMITSS_MRP_TrafficControllerInterface_vissimtime 20000     
####################################################################################
# Applications to run TSP and FSP for Kyoungho in VA
#run_in_bg MMITSS_MRP_MAP_SPAT_Broadcast_ASN 127.0.0.1 127.0.0.1 1
#run_in_bg mehdi_mprsolver_ack
#run_in_bg MMITSS_MRP_PriorityRequestServer_ackasn
#run_in_bg MMITSS_OBE_MAP_SPAT_Receiver_ASN
#run_in_bg MMITSS_OBE_PriorityRequestGenerator_ackasn -c 1


# Do not remove this line
while true; do sleep 1h; done
