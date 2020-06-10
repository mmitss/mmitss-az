#-----------------------------------------------------------------------------#
#    Dockerfile to build an rsu image for an intersection                     #
#-----------------------------------------------------------------------------#
FROM mmitssuarizona/mmitss-base:1.1

MAINTAINER D Cunningham (donaldcunningham@email.arizona.edu)

COPY WirelessMsgDecoder/M_WirelessMsgDecoder /mmitss
COPY TrafficControllerInterface/M_TrafficControllerInterface /mmitss
COPY PrioritySolver/M_PrioritySolver /mmitss
COPY PriorityRequestServer/M_PriorityRequestServer /mmitss
COPY MsgEncoder/M_MsgEncoder /mmitss
COPY HostBsmDecoder/M_HostBsmDecoder /mmitss
COPY MapSpatBroadcaster/M_MapSpatBroadcaster /mmitss

CMD ./M_MapSpatBroadcaster
CMD ./M_HostBsmDecoder
CMD ./M_MsgEncoder
CMD ./M_WirelessMsgDecoder
CMD ./M_PriorityRequestServer
CMD ./M_PrioritySolver
CMD ./M_TrafficControllerInterface