# MMITSS Roadside Processor (MRP):
1) Update existing repositories and upgrade installed packages
        
        sudo apt-get update
        sudo apt-get upgrade        
2) Build-Essential

        sudo apt-get install build-essential
3) SSL library: required to build components that interact with NetSNMP libraries.
      
        sudo apt-get install libssl-dev
4) zlib: required to install pyinstaller

        sudo apt-get install zlib1g-dev
5) PIP3: required to install pyinstaller

        sudo apt-get install python3-pip
6) Pyinstaller: required to build Python components of MMITSS

        sudo pip3 install pyinstaller
7) Chrony: required to manage MRP's system time

        sudo apt-get install chrony
8) Docker: required to containerize MMITSS applications. Follow the instructions provided in [official documentation](https://docs.docker.com/engine/install/ubuntu/) of Docker.
