#-----------------------------------------------------------------------------#
#    Dockerfile to build an x86 platform image for an intersection            #
#-----------------------------------------------------------------------------#
FROM mmitssuarizona/mmitss-base-x86:1.3

COPY TrafficControllerInterface/x86/M_TrafficControllerInterface /mmitss
COPY SnmpEngine/x86/M_SnmpEngine /mmitss
COPY PriorityRequestSolver/x86/M_PriorityRequestSolver /mmitss
COPY PriorityRequestServer/x86/M_PriorityRequestServer /mmitss
COPY MapSpatBroadcaster/x86/M_MapSpatBroadcaster /mmitss

COPY supervisord.conf /mmitss
CMD ["/usr/bin/supervisord"]

