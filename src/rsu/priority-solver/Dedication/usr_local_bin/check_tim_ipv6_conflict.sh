check_tim_config()
{
    timDSRCif=`grep DSRCInterfaceName $TIM_CONFIG | head -1 | cut -d "=" -f 2 |  awk -F ';' '{print $1}' | cut -d '"' -f 2`
    eval timDSRCif="\$timDSRCif"
    timDSRCif=`echo $timDSRCif| cut -d ' ' -f 2`
    timEnable=`grep TIMEnable $TIM_CONFIG | head -1 | cut -d "=" -f 2 | cut -d ";" -f1 | sed -e 's/^ *//g;s/ *$//g'`
    timForcedServChan=`grep ForcedSerChanNum $TIM_CONFIG | grep = | awk -F= '{print $2}' | awk -F";" '{print $1}'`
}

check_ipv6provider_config()
{
    ipv6DSRCif=`grep DSRCInterfaceName $IPV6PROVIDER_CONFIG | head -1 | cut -d "=" -f 2 |  awk -F ';' '{print $1}' | cut -d '"' -f 2`
    eval ipv6DSRCif="\$ipv6DSRCif"
    ipv6DSRCif=`echo $ipv6DSRCif| cut -d ' ' -f 2`
    ipv6Enable=`grep IPV6Enabled $IPV6PROVIDER_CONFIG | head -1 | cut -d "=" -f 2 | cut -d ";" -f1 | sed -e 's/^ *//g;s/ *$//g'`
    ipv6ServChanNum=`grep ServiceChanNum $IPV6PROVIDER_CONFIG | head -1 | cut -d "=" -f 2 | cut -d ";" -f1 | sed -e 's/^ *//g;s/ *$//g'`
}

init()
{

    TIM_CONFIG="/etc/config/TimMain.conf"
    IPV6PROVIDER_CONFIG="/etc/config/ipv6app.conf"

    check_tim_config
    check_ipv6provider_config
}

validatechannels()
{
    init $@

    if [ $timEnable -eq 1 -a $ipv6Enable -eq 1 ]; then
        if [ "$timDSRCif" = "$ipv6DSRCif" ]; then
            if [ $timForcedServChan -ne $ipv6ServChanNum ]; then
                seterror 1
                return 1 # Invalid channels of Tim and Ipv6
            fi
        fi
    fi

    return 0
}

seterror()
{
    err=""
    case $1 in
        1)
            err="Channel conflict in TIM and IPv6"
            ;;
        *)
    esac
    ERROR="$err"
}

case $1 in
        -c) validatechannels
esac
