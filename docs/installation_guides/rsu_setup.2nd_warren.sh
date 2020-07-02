#!/bin/sh
# ddk 20200617

##############################################################################
# User defined variables
##############################################################################
#set -x

SUT_IPV4_ADDR="172.18.49.153"
SUT_IPV4_MASK="255.255.255.192"
SUT_IPV4_BCAST=""
SUT_IPV4_GW="172.18.49.129"
SUT_IPV4_NS=""

#0x0A.0C.06.38 == 172.18.49.152"
WSMFwdRx_ADDR="000000000000000000000000AC123198"
# WSMFwdRx_ADDR="000000000000000000000000C0A8016e"
WSMFwdRx_PORT="10002"
WSMFwdRx_RSSI="-100"
WSMFwdRx_STRT="07E401010000"
WSMFwdRx_STOP="07E801010000"

WSMFwdRx_PSID1="0x20"       #BSM 
WSMFwdRx_PSID2="0x8002"     #SPaT
WSMFwdRx_PSID3="0x8003"     #RSM
WSMFwdRx_PSID4="0xE0000017" #MAP
WSMFwdRx_PSID5="0xE0000019" #SRM
WSMFwdRx_PSID6="0xE0000020" #SSM

#Switch on/off WSM Forwarding
WSMFwdRx_ENABLE1="1"
WSMFwdRx_ENABLE2="0"
WSMFwdRx_ENABLE3="0"
WSMFwdRx_ENABLE4="0"
WSMFwdRx_ENABLE5="1"
WSMFwdRx_ENABLE6="0"

ACT_SVCS_PORT="    Port = 1516;"

ID="rsu"
PW="rsuadmin"
MIB_DIR="/home/duser/vm_share/fw_Release/docs/RSU/mibs/"

##############################################################################
# Setting Environment
##############################################################################

DELAY="sleep 10" 
HOSTNAME=$(cat /etc/hostname)
MICROSD_DIR=$(mount | grep "/dev/mmcblk2p1" | awk '{print $3}')
PARENT_DIR=$(pwd | cut -c-8)

_detect_host()
{
  if [ "$HOSTNAME" == "MKx-SDK" ]; then
    export DIR="$MIB_DIR"
    export IP="udp:$SUT_IPV4_ADDR:161"
  elif [ "$HOSTNAME" == "MK5" ]; then
    export DIR="/mnt/rw/rsu1609/snmp/mibs/"
    export IP="udp:127.0.0.1:161"
  else
    echo "Host not recognized"
    exit 0
  fi
  export RW_AUTH_ARGS="-t 15 -v3 -lauthPriv -M $DIR -m RSU-MIB -u $ID -A $PW -X $PW -aSHA -xAES $IP"
}

_enforce_run_from_microSD()
{
  if [ "$MICROSD_DIR" != "$PARENT_DIR" ]; then
    echo "Closing... Please run from microSD card, directory /mnt/src/*"
    exit 0
  fi
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
  echo "WBSS_Service_Mode       = 0"   >> /mnt/rw/rsu1609/conf/stack.conf

: <<'SKIP0'
  echo "BSMEnabled              = 1"    >> /mnt/rw/rsu1609/conf/stack.conf
  echo "HeadingUseDefault       = 1"    >> /mnt/rw/rsu1609/conf/stack.conf 
  echo "BSMUnsecurePSID         = 0x20" >> /mnt/rw/rsu1609/conf/stack.conf
  echo "RandMAC                 = 0"    >> /mnt/rw/rsu1609/conf/stack.conf
  echo "WSMP_TxPower            = 24"   >> /mnt/rw/rsu1609/conf/stack.conf
SKIP0

  echo "WSMP_ChannelNumber      = 172" >> /mnt/rw/rsu1609/conf/stack.conf
  echo "ContinuousChanNum       = 172" >> /mnt/rw/rsu1609/conf/stack.conf
  echo "ForcedSerChanNum        = 172" >> /mnt/rw/rsu1609/conf/stack.conf
  echo "ForcedControlChanNum    = 172" >> /mnt/rw/rsu1609/conf/stack.conf
}

_unfiltered_packet_forwarding()
{
  if [ -e $PWD/rc.mcap ]; then
    chmod 777 $PWD/rc.mcap
    cp -rfp $PWD/rc.mcap /mnt/rw/rc.mcap
    rm -rf /mnt/rw/rc.local 1>&2>/dev/null
    #ln -s /mnt/rw/rc.mcap /mnt/rw/rc.local 
    /mnt/rw/rc.mcap start 1>&2>/dev/null 
  else
    echo "File rc.mcap not found.  Proceeding anyway..."
  fi
}

_edit_rsu_cfg()
{
  sed -i "57s/.*/$ACT_SVCS_PORT/" /mnt/rw/rsu1609/conf/rsu.cfg
}

_manually_manipulate_rsu_files()
{
if [ "$HOSTNAME" == "MK5" ]; then
  echo
  echo "Performing manual setup that cannot be accomplished via SNMP"
  _set_static_ipv4_eth0

  echo "stopping application(s)"
  /opt/cohda/application/rc.local stop 1>&2>/dev/null 
  /mnt/rw/rc.local stop 1>&2>/dev/null 

  dmesg -c 1>&2>/dev/null  
  net-snmp-config --create-snmpv3-user -A $PW -X $PW -a SHA -x AES $ID

  _edit_stack_conf
  #_edit_rsu_cfg
  rm -rf /mnt/rw/rsu1609/conf/user.conf 
  #_unfiltered_packet_forwarding
  #_setup_coredump
  sync

  echo "starting application(s)"
  /opt/cohda/application/rc.local start 1>&2>/dev/null 
  /mnt/rw/rc.local start 1>&2>/dev/null 

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
# WSMFwdRx_* table
##############################################################################
_destroy_WSMFwdRx_Table()
{
echo
echo "SNMP: Destroy rsuDsrcFwd table"
snmpset $RW_AUTH_ARGS \
rsuDsrcFwdStatus.9 i 6 \
rsuDsrcFwdStatus.8 i 6 \
rsuDsrcFwdStatus.7 i 6 \
rsuDsrcFwdStatus.6 i 6 \
rsuDsrcFwdStatus.5 i 6 \
rsuDsrcFwdStatus.4 i 6 \
rsuDsrcFwdStatus.3 i 6 \
rsuDsrcFwdStatus.2 i 6 \
rsuDsrcFwdStatus.1 i 6
}

_set_WSMFwdRx1()
{
echo
echo "SNMP: Set WSMFwd_Rx_*" 
snmpset $RW_AUTH_ARGS \
rsuDsrcFwdPsid.1 x "$WSMFwdRx_PSID1" \
rsuDsrcFwdDestIpAddr.1 x "$WSMFwdRx_ADDR" \
rsuDsrcFwdDestPort.1 i "$WSMFwdRx_PORT" \
rsuDsrcFwdProtocol.1 i 2 \
rsuDsrcFwdRssi.1 i "$WSMFwdRx_RSSI" \
rsuDsrcFwdMsgInterval.1 i 1 \
rsuDsrcFwdDeliveryStart.1 x "$WSMFwdRx_STRT" \
rsuDsrcFwdDeliveryStop.1 x "$WSMFwdRx_STOP" \
rsuDsrcFwdEnable.1 i "$WSMFwdRx_ENABLE1" \
rsuDsrcFwdStatus.1 i 4
}

_set_WSMFwdRx2()
{
echo
echo "SNMP: Set WSMFwd_Rx_*" 
snmpset $RW_AUTH_ARGS \
rsuDsrcFwdPsid.2 x "$WSMFwdRx_PSID2" \
rsuDsrcFwdDestIpAddr.2 x "$WSMFwdRx_ADDR" \
rsuDsrcFwdDestPort.2 i "$WSMFwdRx_PORT" \
rsuDsrcFwdProtocol.2 i 2 \
rsuDsrcFwdRssi.2 i "$WSMFwdRx_RSSI" \
rsuDsrcFwdMsgInterval.2 i 1 \
rsuDsrcFwdDeliveryStart.2 x "$WSMFwdRx_STRT" \
rsuDsrcFwdDeliveryStop.2 x "$WSMFwdRx_STOP" \
rsuDsrcFwdEnable.2 i "$WSMFwdRx_ENABLE2" \
rsuDsrcFwdStatus.2 i 4
}

_set_WSMFwdRx3()
{
echo
echo "SNMP: Set WSMFwd_Rx_*" 
snmpset $RW_AUTH_ARGS \
rsuDsrcFwdPsid.3 x "$WSMFwdRx_PSID3" \
rsuDsrcFwdDestIpAddr.3 x "$WSMFwdRx_ADDR" \
rsuDsrcFwdDestPort.3 i "$WSMFwdRx_PORT" \
rsuDsrcFwdProtocol.3 i 2 \
rsuDsrcFwdRssi.3 i "$WSMFwdRx_RSSI" \
rsuDsrcFwdMsgInterval.3 i 1 \
rsuDsrcFwdDeliveryStart.3 x "$WSMFwdRx_STRT" \
rsuDsrcFwdDeliveryStop.3 x "$WSMFwdRx_STOP" \
rsuDsrcFwdEnable.3 i "$WSMFwdRx_ENABLE3" \
rsuDsrcFwdStatus.3 i 4
}

_set_WSMFwdRx4()
{
echo
echo "SNMP: Set WSMFwd_Rx_*" 
snmpset $RW_AUTH_ARGS \
rsuDsrcFwdPsid.4 x "$WSMFwdRx_PSID4" \
rsuDsrcFwdDestIpAddr.4 x "$WSMFwdRx_ADDR" \
rsuDsrcFwdDestPort.4 i "$WSMFwdRx_PORT" \
rsuDsrcFwdProtocol.4 i 2 \
rsuDsrcFwdRssi.4 i "$WSMFwdRx_RSSI" \
rsuDsrcFwdMsgInterval.4 i 1 \
rsuDsrcFwdDeliveryStart.4 x "$WSMFwdRx_STRT" \
rsuDsrcFwdDeliveryStop.4 x "$WSMFwdRx_STOP" \
rsuDsrcFwdEnable.4 i "$WSMFwdRx_ENABLE4" \
rsuDsrcFwdStatus.4 i 4
}

_set_WSMFwdRx5()
{
echo
echo "SNMP: Set WSMFwd_Rx_*" 
snmpset $RW_AUTH_ARGS \
rsuDsrcFwdPsid.5 x "$WSMFwdRx_PSID5" \
rsuDsrcFwdDestIpAddr.5 x "$WSMFwdRx_ADDR" \
rsuDsrcFwdDestPort.5 i "$WSMFwdRx_PORT" \
rsuDsrcFwdProtocol.5 i 2 \
rsuDsrcFwdRssi.5 i "$WSMFwdRx_RSSI" \
rsuDsrcFwdMsgInterval.5 i 1 \
rsuDsrcFwdDeliveryStart.5 x "$WSMFwdRx_STRT" \
rsuDsrcFwdDeliveryStop.5 x "$WSMFwdRx_STOP" \
rsuDsrcFwdEnable.5 i "$WSMFwdRx_ENABLE5" \
rsuDsrcFwdStatus.5 i 4
}

_set_WSMFwdRx6()
{
echo
echo "SNMP: Set WSMFwd_Rx_*" 
snmpset $RW_AUTH_ARGS \
rsuDsrcFwdPsid.6 x "$WSMFwdRx_PSID6" \
rsuDsrcFwdDestIpAddr.6 x "$WSMFwdRx_ADDR" \
rsuDsrcFwdDestPort.6 i "$WSMFwdRx_PORT" \
rsuDsrcFwdProtocol.6 i 2 \
rsuDsrcFwdRssi.5 i "$WSMFwdRx_RSSI" \
rsuDsrcFwdMsgInterval.6 i 1 \
rsuDsrcFwdDeliveryStart.6 x "$WSMFwdRx_STRT" \
rsuDsrcFwdDeliveryStop.6 x "$WSMFwdRx_STOP" \
rsuDsrcFwdEnable.6 i "$WSMFwdRx_ENABLE6" \
rsuDsrcFwdStatus.6 i 4
}
_get_WSMFwdRx1()
{
echo
echo "SNMP: Get WSMFwd_Rx_*" 
snmpget $RW_AUTH_ARGS \
rsuDsrcFwdPsid.1 \
rsuDsrcFwdDestIpAddr.1 \
rsuDsrcFwdDestPort.1 \
rsuDsrcFwdProtocol.1 \
rsuDsrcFwdRssi.1 \
rsuDsrcFwdMsgInterval.1 \
rsuDsrcFwdDeliveryStart.1 \
rsuDsrcFwdDeliveryStop.1 \
rsuDsrcFwdEnable.1
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


##############################################################################
##############################################################################
# Main
##############################################################################

echo
echo "Initial test"
_enforce_run_from_microSD
_detect_host
_manually_manipulate_rsu_files

_set_standby
_set_operate

echo
echo "SNMP: Enable pcap logging and latch"
_set_standby
_enable_pcap_logging
_set_operate

_set_standby
_destroy_WSMFwdRx_Table
_set_operate

_set_standby
_set_WSMFwdRx1
_set_WSMFwdRx2
_set_WSMFwdRx3
_set_WSMFwdRx4
_set_WSMFwdRx5
_set_WSMFwdRx6
_set_operate
_get_WSMFwdRx1

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

: <<'SKIP1'
SKIP1

exit 0
