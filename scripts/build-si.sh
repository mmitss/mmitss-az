#!/bin/bash

# Define colors:
red='\033[0;31m'
green='\033[0;32m'
nocolor='\033[0m'

#######################################################################################
echo "Building System Interface..."
cd ./../src/system-interface
# Clean the folder and build for linux.
pyinstaller --add-data "templates:templates" --add-data "static:static" --additional-hooks-dir=. --onefile --windowed system-interface.py &> /dev/null
# Indicate Success/Failure of the build
if [ "$?" -eq "0" ]; then
    mv dist/system-interface  ../../bin/SystemInterface/arm/M_SystemInterface
	echo -e "${green}Successful${nocolor}"
else
	echo -e "${red}Failed${nocolor}"
fi
# Remove the files to keep the folders clean
rm -r build dist *.spec &> /dev/null
rm -r __pycache__ &> /dev/null
# Return back to original directory to go over the process again for another one
cd - &> /dev/null
sleep 1s
#######################################################################################
