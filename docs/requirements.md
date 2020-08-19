# MMITSS Roadside Processor (MRP):
1) Update existing repositories and upgrade installed packages
        
        sudo apt-get update
        sudo apt-get upgrade        
2) Build-Essential

        sudo apt-get install build-essential
3) SSL library: this library is required to build components that interact with NetSNMP libraries.
      
        sudo apt-get install libssl-dev
4) zlib: this library is required to install pyinstaller

        sudo apt-get install zlib1g-dev
5) PIP3: required to install pyinstaller

        sudo apt-get install python3-pip
6) Pyinstaller: a tool used to build Python components of MMITSS

        sudo pip3 install pyinstaller
7) Chrony: a tool used to manage MRP's system time

        sudo apt-get install chrony
