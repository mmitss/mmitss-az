# docker-compose

This directory is a warehouse of docker-compose files customized for two types of deployment environments:
- Server-based deployment, where containers of multiple intersection run on a single machine.
- Simulation deployment, where containers of multiple simulated intersections, along with a container hosting simulation tools run on a single machine.

To use any of the available docker-compose files under this directory, following environment variables must be added to the ~/.bashrc file:
1. MMITSS_ROOT: Local path of the directory where the mmitss-az git repository is cloned.
2. MMITSS_NETWORK_ADAPTER: Name of the network adapter that will be used by MMITSS containers. This can be obtained from running command `ifconfig` in the console. For example, an output of the `ifconfig` command is shown below, where the name of the network adapter is highlighted with bold text.
<pre>
docker0: flags=4099<UP,BROADCAST,MULTICAST>  mtu 1500
        inet 172.17.0.1  netmask 255.255.0.0  broadcast 172.17.255.255
        inet6 fe80::42:61ff:fe79:ad2  prefixlen 64  scopeid 0x20<link>
        ether 02:42:61:79:0a:d2  txqueuelen 0  (Ethernet)
        RX packets 0  bytes 0 (0.0 B)
        RX errors 0  dropped 0  overruns 0  frame 0
        TX packets 11  bytes 1366 (1.3 KB)
        TX errors 0  dropped 0 overruns 0  carrier 0  collisions 0

<b>enp0s31f6</b>: flags=4163<UP,BROADCAST,RUNNING,MULTICAST>  mtu 1500,
        inet 10.12.6.252  netmask 255.255.255.0  broadcast 10.12.6.255
        inet6 fe80::b417:3aa0:cfac:74fc  prefixlen 64  scopeid 0x20<link>
        ether 64:00:6a:7d:15:c5  txqueuelen 1000  (Ethernet)
        RX packets 1883174  bytes 166606949 (166.6 MB)
        RX errors 0  dropped 0  overruns 0  frame 0
        TX packets 22387  bytes 3306725 (3.3 MB)
        TX errors 0  dropped 0 overruns 0  carrier 0  collisions 0
        device interrupt 19  memory 0xf7e80000-f7ea0000  
        
lo: flags=73<UP,LOOPBACK,RUNNING>  mtu 65536
        inet 127.0.0.1  netmask 255.0.0.0
        inet6 ::1  prefixlen 128  scopeid 0x10<host>
        loop  txqueuelen 1000  (Local Loopback)
        RX packets 26009  bytes 2720630 (2.7 MB)
        RX errors 0  dropped 0  overruns 0  frame 0
        TX packets 26009  bytes 2720630 (2.7 MB)
        TX errors 0  dropped 0 overruns 0  carrier 0  collisions 0

wlp2s0: flags=4163<UP,BROADCAST,RUNNING,MULTICAST>  mtu 1500
        inet 192.168.1.20  netmask 255.255.255.0  broadcast 192.168.1.255
        inet6 fe80::cd1c:d7af:981b:64fa  prefixlen 64  scopeid 0x20<link>
        ether bc:54:2f:d2:0f:32  txqueuelen 1000  (Ethernet)
        RX packets 460409  bytes 449341593 (449.3 MB)
        RX errors 0  dropped 0  overruns 0  frame 0
        TX packets 168502  bytes 28321239 (28.3 MB)
        TX errors 0  dropped 0 overruns 0  carrier 0  collisions 0
</pre>
3. PROCESSOR: The architecture of the processor. This should take either of the following values:
    - arm
    - x86

Above requirements can be automatically satisfied by using the `setup-build-environment.sh` script located in the `mmitss/scripts` directory.
