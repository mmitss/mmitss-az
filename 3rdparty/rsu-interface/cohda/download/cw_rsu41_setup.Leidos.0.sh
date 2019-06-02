#!/bin/sh
# ddk 201900516

##############################################################################
# User defined variables
##############################################################################

SUT_IPV4_ADDR="192.168.1.127"
SUT_IPV4_MASK="255.255.255.0"
SUT_IPV4_BCAST=""
SUT_IPV4_GW="" 
SUT_IPV4_NS=""

SUT_IPV6_ETH0=""
SUT_IPV6_WAVE=""
SUT_IPV6_GW=""
SUT_IPV6_SUB=""
SUT_IPV6_NS=""

WSMFWDRX_0_DESTIP="fe80::6e5:48ff:fe01:3298"
WSMFWDRX_0_DESTPORT="4445"
WSMFWDRX_0_PSID="0x7"
WSMFWDRX_1_DESTIP="fe80::6e5:48ff:fe01:3298"
WSMFWDRX_1_DESTPORT="445"
WSMFWDRX_1_PSID="0x20"

#192.168.1.72 == 0xC0.0xA8.0x01.0x48
FW_NMEA_ADDR="0x000000000000000000000000C0A80148"
FW_NMEA_PORT="2021"

FW_DSRC_ADDR1="0x000000000000000000000000C0A80148"
FW_DSRC_PORT1="2022"
FW_DSRC_PSID1="0x20"
FW_DSRC_RSSI1="-100"

ACT_SVCS_PORT="    Port = 1516;"

ID="rsu"
PW="rsuadmin"
DELAY="sleep 10" 
MIB_DIR="/home/duser/vm_share/fw_Release/docs/RSU/mibs/"

##############################################################################
# Setting Environment
##############################################################################

_detect_host()
{
  cat /etc/hostname > HOSTNAME

  if [ "$HOSTNAME" == "MKx-SDK" ]; then
    export DIR="$MIB_DIR"
    export IP="udp:$SUT_IPV4_ADDR:161"
  elif [ "$HOSTNAME" == "MK5" ]; then
    export DIR="/mnt/rw/rsu1609/snmp/mibs/"
    #export IP="udp:127.0.0.1:161"
    export IP="udp6:[::1]:161"
  else
    echo "Host not recognized"
    exit 0
  fi
  export RW_AUTH_ARGS="-t 15 -v3 -lauthPriv -M $DIR -m RSU-MIB -u $ID -A $PW -X $PW -aSHA -xAES $IP"
}


##############################################################################
# Local setup (file manipulation and settings)
##############################################################################

_set_static_ipv4_eth0()
{
  read -n1 -p "Assign static ip $SUT_IPV4_ADDR?  Then press y" set_static_ip
  if [ "$set_static_ip" = "y" ]; then
    fw_setenv static_ip_addr  $SUT_IPV4_ADDR
    fw_setenv static_ip_mask  $SUT_IPV4_MASK
    fw_setenv static_ip_bcast $SUT_IPV4_BCAST
    fw_setenv static_ip_gw    $SUT_IPV4_GW
    fw_setenv static_ip_ns    $SUT_IPV4_NS

    sync
    echo ""
    echo "Rebooting, to enable static addressing"
    reboot
  else echo ""
    echo "No change in IPv4 address:"
    ifconfig eth0 | egrep 'inet|Link'
  fi
}

_setup_coredump()
{
  echo
  echo "Setting up coredump"
  sed  -i '/  "start")/ a \ \ \ \ ulimit -c unlimited' /opt/cohda/application/rsu1609/rc.rsu1609
  sync
}

_edit_stack_conf()
{

  echo "Cohda_DebugLevel        = 4"   >> /mnt/rw/rsu1609/conf/stack.conf
  echo "SecurityEnable          = 0"   >> /mnt/rw/rsu1609/conf/stack.conf
  echo "SendUnsecuredDot2Header = 1"   >> /mnt/rw/rsu1609/conf/stack.conf
  echo "WBSS_WSA_RepeatRate     = 5"   >> /mnt/rw/rsu1609/conf/stack.conf 
  echo "WBSS_Service_Mode       = 0"   >> /mnt/rw/rsu1609/conf/stack.conf 

  echo "WSMFwdRx_Enabled    = 1"                    >> /mnt/rw/rsu1609/conf/stack.conf
  echo "WSMFwdRx_0_Enabled  = 1"                    >> /mnt/rw/rsu1609/conf/stack.conf 
  echo "WSMFwdRx_0_DestIP   = $WSMFWDRX_0_DESTIP"   >> /mnt/rw/rsu1609/conf/stack.conf
  echo "WSMFwdRx_0_DestPort = $WSMFWDRX_0_DESTPORT" >> /mnt/rw/rsu1609/conf/stack.conf
  echo "WSMFwdRx_0_PSID     = $WSMFWDRX_0_PSID"     >> /mnt/rw/rsu1609/conf/stack.conf
  echo "WSMFwdRx_1_Enabled  = 1"                    >> /mnt/rw/rsu1609/conf/stack.conf 
  echo "WSMFwdRx_1_DestIP   = $WSMFWDRX_1_DESTIP"   >> /mnt/rw/rsu1609/conf/stack.conf
  echo "WSMFwdRx_1_DestPort = $WSMFWDRX_1_DESTPORT" >> /mnt/rw/rsu1609/conf/stack.conf
  echo "WSMFwdRx_1_PSID     = $WSMFWDRX_1_PSID"     >> /mnt/rw/rsu1609/conf/stack.conf

  echo "WBSS_Service_2_ChanId   = "     >> /mnt/rw/rsu1609/conf/stack.conf
  echo "WBSS_Service_2_PSID     = 0xff" >> /mnt/rw/rsu1609/conf/stack.conf
  echo "WBSS_Service_2_PSC      = "     >> /mnt/rw/rsu1609/conf/stack.conf
  echo "WBSS_Service_3_ChanId   = "     >> /mnt/rw/rsu1609/conf/stack.conf
  echo "WBSS_Service_3_PSID     = 0xff" >> /mnt/rw/rsu1609/conf/stack.conf
  echo "WBSS_Service_3_PSC      = "     >> /mnt/rw/rsu1609/conf/stack.conf
}

_edit_rsu_cfg()
{
  #sed -i "s/1516/1510/g" /mnt/rw/rsu1609/conf/rsu.cfg
  sed -i "57s/.*/$ACT_SVCS_PORT/" /mnt/rw/rsu1609/conf/rsu.cfg
}

_manually_manipulate_rsu_files()
{
if [ "$HOSTNAME" == "MK5" ]; then
  echo
  echo "Performing manual setup that cannot be accomplished via SNMP"
  _set_static_ipv4_eth0
  /opt/cohda/application/rc.local stop
  dmesg -c 2>/dev/null
  net-snmp-config --create-snmpv3-user -A $PW -X $PW -a SHA -x AES $ID

  _edit_stack_conf
  _edit_rsu_cfg
  rm -rf /mnt/rw/rsu1609/conf/user.conf 
#  _setup_coredump
  sync

  /opt/cohda/application/rc.local start
  echo
  echo "$DELAY after restart"
  $DELAY 
fi
}


##############################################################################
# Helper Functions
##############################################################################

_set_standby()
{
  snmpset $RW_AUTH_ARGS rsuMode.0 i 2
  until snmpget $RW_AUTH_ARGS rsuMode.0 | grep -q 'standby(2)'; do
  #date +%s
  echo "Waiting for confirmation..."
  sleep 1
  done
}

_set_operate()
{
  snmpset $RW_AUTH_ARGS rsuMode.0 i 4
  until snmpget $RW_AUTH_ARGS rsuMode.0 | grep -q 'operate(4)'; do
  #date +%s
  echo "Waiting for confirmation..."
  sleep 1
  done
}

_enable_pcap_logging()
{
  #txa (.2)
  #rxa (.3)
  #txb (.4)
  #rxb (.5)
  snmpset $RW_AUTH_ARGS \
  rsuIfaceGenerate.2 i  1 \
  rsuIfaceGenerate.3 i  1 \
  rsuIfaceGenerate.4 i  1 \
  rsuIfaceGenerate.5 i  1 \
  rsuIfaceMaxFileSize.2 i 40 \
  rsuIfaceMaxFileSize.3 i 40 \
  rsuIfaceMaxFileSize.4 i 40 \
  rsuIfaceMaxFileSize.5 i 40
}


##############################################################################
# Store-and-Forward table 
##############################################################################
#define DOT3_WSMP_PSID_4BYTE_MAX     0xEFFFFFFF
#define DOT3_WSMP_PSID_4BYTE_MIN     0xE0000000
#define DOT3_WSMP_PSID_3BYTE_MAX     0xDFFFFF
#define DOT3_WSMP_PSID_3BYTE_MIN     0xC00000
#define DOT3_WSMP_PSID_2BYTE_MAX     0xBFFF
#define DOT3_WSMP_PSID_2BYTE_MIN     0x8000
#define DOT3_WSMP_PSID_1BYTE_MAX     0x7F
#define DOT3_WSMP_PSID_1BYTE_MIN     0x00

_destroy_StoreAndForward()
{
echo
echo "SNMP: Destroy StoreAndForward Table"
snmpset $RW_AUTH_ARGS \
rsuSRMStatus.2 i 6 \
rsuSRMStatus.1 i 6
}

_set_StoreAndForward1()
{
echo
echo "SNMP: Set StoreAndForward table entry"
snmpset $RW_AUTH_ARGS \
rsuSRMPsid.1 x 8002 \
rsuSRMDsrcMsgId.1 i 20181210 \
rsuSRMTxMode.1 i 0 \
rsuSRMTxChannel.1 i 172 \
rsuSRMTxInterval.1 i 1000 \
rsuSRMDeliveryStart.1 x 07E201010000 \
rsuSRMDeliveryStop.1 x 07E401010000 \
rsuSRMPayload.1 x 001281103806300021a1d0194dbca410377a62a215fa02dc0ed0080ce8488000807600018dde98c3a6de4fe318dde98e526de4d79b401035e112000201d80006377a69a79b794c286377a734b9b79490ab020445857e2e7e5979f30200040200000c6ef4d31b36f2907cc6ef4e4e936f2897960488a70afc5cfcb2f3e604000804000018dde984ae6de4fde18dde9865a6de4d50b4030316132000201d80006377a5bee9b7948e26377a53499b7948e2b018457857e2e7e5979f30200040200000c6ef4b73b36f297d4c6ef4a6ad36f29731a0201a70a1000100ec00071bbd30b74dbca93a02828c6ef4c2f736f2bc85603889d0afc5cfcb2f3e604000804000038dde98ca26de54c6014146377a63439b795e1a \
rsuSRMEnable.1 i 1 \
rsuSRMStatus.1 i 4
}

_get_StoreAndForward1()
{
echo
echo "SNMP: Get StoreAndForward table entry"
snmpget $RW_AUTH_ARGS \
rsuSRMPsid.1 \
rsuSRMDsrcMsgId.1 \
rsuSRMTxMode.1 \
rsuSRMTxChannel.1 \
rsuSRMTxInterval.1 \
rsuSRMDeliveryStart.1 \
rsuSRMDeliveryStop.1 \
rsuSRMPayload.1 \
rsuSRMEnable.1 
}

##############################################################################
##############################################################################
# Main
##############################################################################

echo
echo "Initial test"
_detect_host
_manually_manipulate_rsu_files
_set_standby
_set_operate

echo
echo "SNMP: Enable pcap logging and latch"
_set_standby
_enable_pcap_logging
_set_operate

: <<'SKIP0'
_set_standby
_destroy_StoreAndForward
_set_operate

_set_standby
_set_StoreAndForward1
_set_operate
_get_StoreAndForward1
SKIP0

echo
echo "SNMP: 'Walk' the entries"
_set_standby
snmpwalk $RW_AUTH_ARGS iso.0.15628.4.1.1
snmpwalk $RW_AUTH_ARGS iso.0.15628.4.1.2
snmpwalk $RW_AUTH_ARGS iso.0.15628.4.1.3
snmpwalk $RW_AUTH_ARGS iso.0.15628.4.1.4
#snmpwalk $RW_AUTH_ARGS iso.0.15628.4.1.5
snmpwalk $RW_AUTH_ARGS iso.0.15628.4.1.6
snmpwalk $RW_AUTH_ARGS iso.0.15628.4.1.7
#snmpwalk $RW_AUTH_ARGS iso.0.15628.4.1.8
snmpwalk $RW_AUTH_ARGS iso.0.15628.4.1.9
snmpwalk $RW_AUTH_ARGS iso.0.15628.4.1.10
snmpwalk $RW_AUTH_ARGS iso.0.15628.4.1.11
snmpwalk $RW_AUTH_ARGS iso.0.15628.4.1.12
snmpwalk $RW_AUTH_ARGS iso.0.15628.4.1.13
snmpwalk $RW_AUTH_ARGS iso.0.15628.4.1.14
snmpwalk $RW_AUTH_ARGS iso.0.15628.4.1.15
snmpwalk $RW_AUTH_ARGS iso.0.15628.4.1.16
snmpwalk $RW_AUTH_ARGS iso.0.15628.4.1.17
snmpwalk $RW_AUTH_ARGS iso.0.15628.4.1.18
snmpwalk $RW_AUTH_ARGS iso.0.15628.4.1.19
_set_operate

exit 0
