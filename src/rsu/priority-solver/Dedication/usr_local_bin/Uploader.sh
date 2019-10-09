remove_file()
{
    rm -rf $filename
    sync
}

use_certs()
{
filename=$1;
ver=$2
user=$3;
local=$4;
ip=$5;
filepath=$6;

    if [ $ver = 4 ] ; then
        scp -l 2048 -o StrictHostKeyChecking=no -o PasswordAuthentication=no $filename $user@$ip:$filepath 2>/dev/null 1>/dev/null
    else
        if [ $local = 1 ]; then
            scp -l 2048 -o StrictHostKeyChecking=no -o PasswordAuthentication=no $filename $user@\[$ip%"eth0"\]:$filepath 2>/dev/null 1>/dev/null
        else
            scp -l 2048 -o StrictHostKeyChecking=no -o PasswordAuthentication=no $filename $user@\[$ip\]:$filepath 2>/dev/null 1>/dev/null
        fi
    fi
    ret=`echo $?`
    if [ $ret -eq 0 ]; then
        remove_file
    fi
    return $ret
}

use_passwd()
{
passwd=$1;
filename=$2;
ver=$3;
user=$4;
local=$5;
ip=$6;
filepath=$7;
ifr="eth0";

    if [ $ver = 4 ] ; then
        sshpass -p "$passwd" scp -l 2048 -o StrictHostKeyChecking=no $filename $user@$ip:$filepath 2>/dev/null 1>/dev/null 
    else
        if [ $local = 1 ] ; then
            sshpass -p "$passwd" scp -l 2048 -o StrictHostKeyChecking=no $filename $user@\[$ip%"eth0"\]:$filepath 2>/dev/null 1>/dev/null
        else
            sshpass -p "$passwd" scp -l 2048 -o StrictHostKeyChecking=no $filename $user@\[$ip\]:$filepath 2>/dev/null 1>/dev/null
        fi
    fi
    ret=`echo $?`
    if [ $ret -eq 0 ]; then
        remove_file
    fi
    return $ret
}

case $1 in 
 -p) use_passwd $2 $3 $4 $5 $6 $7 $8;;
 -S) use_certs $2 $3 $4 $5 $6 $7;;
esac
