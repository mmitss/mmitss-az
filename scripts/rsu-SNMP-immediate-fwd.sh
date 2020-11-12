#!/bin/bash

USER="rsuRwUser"
PASS="C2xsag!!"
RSU_MIB="iso.0.15628.4.1"
IFM_PORT=1516

RSU_IP="10.12.6.19"
RSU_ADDR="udp:${RSU_IP}:161"
RW_AUTH_ARGS="-t 2 -v 3 -l authPriv -a SHA -A ${PASS} -x AES -X ${PASS} -u ${USER}"

#snmpset ${RW_AUTH_ARGS} ${RSU_ADDR} ${RSU_MIB}.5.1.7.1 i 6
#snmpset ${RW_AUTH_ARGS} ${RSU_ADDR} ${RSU_MIB}.5.1.7.2 i 6


snmpset ${RW_AUTH_ARGS} ${RSU_ADDR} \
  ${RSU_MIB}.5.1.2.1 x 8002 \
  ${RSU_MIB}.5.1.3.1 i 1 \
  ${RSU_MIB}.5.1.4.1 i 0 \
  ${RSU_MIB}.5.1.5.1 i 172 \
  ${RSU_MIB}.5.1.6.1 i 1 \
  ${RSU_MIB}.5.1.7.1 i 4

#snmpset ${RW_AUTH_ARGS} ${RSU_ADDR} \
#  ${RSU_MIB}.5.1.2.2 x 0020 \
#  ${RSU_MIB}.5.1.3.2 i 1 \
#  ${RSU_MIB}.5.1.4.2 i 0 \
#  ${RSU_MIB}.5.1.5.2 i 172 \
#  ${RSU_MIB}.5.1.6.2 i 1 \
#  ${RSU_MIB}.5.1.7.2 i 4
