#-----------------------------------------------------------------------------#
#    Dockerfile to build an x86 platform image for an intersection            #
#-----------------------------------------------------------------------------#
FROM mmitssuarizona/mmitss-base:1.2

COPY WirelessMsgDecoder/x86/M_WirelessMsgDecoder /mmitss
COPY TrafficControllerInterface/x86/M_TrafficControllerInterface /mmitss
COPY SnmpEngine/x86/M_SnmpEngine /mmitss
COPY PriorityRequestSolver/x86/M_PriorityRequestSolver /mmitss
COPY PriorityRequestServer/x86/M_PriorityRequestServer /mmitss
COPY MsgEncoder/x86/M_MsgEncoder /mmitss
COPY MapSpatBroadcaster/x86/M_MapSpatBroadcaster /mmitss

COPY supervisord.conf /mmitss
CMD ["/usr/bin/supervisord"]

