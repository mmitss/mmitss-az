#!/bin/bash

USER="rsuRwUser"
PASS="C2xsag!!"
RSU_MIB="iso.0.15628.4.1"
IFM_PORT=1516
OWN_IP=172.24.5.250

SUT_IP="172.24.5.254"
SUT_ADDR="udp:${SUT_IP}:161"
RW_AUTH_ARGS="-t 2 -v 3 -l authPriv -a SHA-512 -A ${PASS} -x AES256 -X ${PASS} -u ${USER}"

set_date() { # arg is additional minutes to add to the current time/date
    local DS_YEAR
    local DS_MON
    local DS_DAY
    local DS_HR
    local DS_MIN
    printf -v DS_YEAR '%04x' $((10#`date -u +'%Y'`))
    printf -v DS_MON '%02x' $((10#`date -u +'%m'`))
    printf -v DS_DAY '%02x' $((10#`date -u +'%d'`))
    printf -v DS_HR '%02x' $((10#`date -u +'%H'`))
    printf -v DS_MIN '%02x' $((10#`date -u +'%M' -d "$1"`))
    echo $(echo $DS_YEAR$DS_MON$DS_DAY$DS_HR$DS_MIN)
}

IDX=1
PSID=0x8002
PROTOCOL=2
RSSI=-100
MSG_INT=1
IP=00000000000000000000ffffac1805fa
PORT=10002
DATE_START=$(set_date "+0 min" )
DATE_END=07F30A1C1533


#clear table if exists
snmpset ${RW_AUTH_ARGS} ${SUT_ADDR} ${RSU_MIB}.7.1.11.${IDX} i 6

echo "Set DSRC forward PSID=${PSID} to ${OWN_IP}"
snmpset ${RW_AUTH_ARGS} ${SUT_ADDR} \
  ${RSU_MIB}.7.1.2.${IDX} x ${PSID} \
  ${RSU_MIB}.7.1.3.${IDX} x ${IP} \
  ${RSU_MIB}.7.1.4.${IDX} i ${PORT} \
  ${RSU_MIB}.7.1.5.${IDX} i ${PROTOCOL} \
  ${RSU_MIB}.7.1.6.${IDX} i ${RSSI} \
  ${RSU_MIB}.7.1.7.${IDX} i ${MSG_INT}\
  ${RSU_MIB}.7.1.8.${IDX} x ${DATE_START} \
  ${RSU_MIB}.7.1.9.${IDX} x ${DATE_END} \
  ${RSU_MIB}.7.1.10.${IDX} i 1 \
  ${RSU_MIB}.7.1.11.${IDX} i 4

IDX=2
PSID=0x0020

#clear table if exists
snmpset ${RW_AUTH_ARGS} ${SUT_ADDR} ${RSU_MIB}.7.1.11.${IDX} i 6

echo "Set DSRC forward PSID=${PSID} to ${OWN_IP}"
snmpset ${RW_AUTH_ARGS} ${SUT_ADDR} \
  ${RSU_MIB}.7.1.2.${IDX} x ${PSID} \
  ${RSU_MIB}.7.1.3.${IDX} x ${IP} \
  ${RSU_MIB}.7.1.4.${IDX} i ${PORT} \
  ${RSU_MIB}.7.1.5.${IDX} i ${PROTOCOL} \
  ${RSU_MIB}.7.1.6.${IDX} i ${RSSI} \
  ${RSU_MIB}.7.1.7.${IDX} i ${MSG_INT}\
  ${RSU_MIB}.7.1.8.${IDX} x ${DATE_START} \
  ${RSU_MIB}.7.1.9.${IDX} x ${DATE_END} \
  ${RSU_MIB}.7.1.10.${IDX} i 1 \
  ${RSU_MIB}.7.1.11.${IDX} i 4

#snmpwalk  ${RW_AUTH_ARGS} ${SUT_ADDR} ${RSU_MIB}
