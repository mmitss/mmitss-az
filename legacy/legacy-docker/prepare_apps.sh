#!/bin/bash
# This script finds all the Makefiles present in the rse sources, builds that 
# project for linux and copies it to the applications folder in this directory

red='\033[0;31m'

green='\033[0;32m'
nocolor='\033[0m'

if [ ! -d "../src/rsu" ] || [ ! -d "../src/obu" ]; then
	echo "All application sources not available."
	exit 1
fi

appdir=$PWD/applications
mkdir -p $appdir

# list all the folders that contain a Makefile
for i in $(find ../src/rsu -name Makefile) $(find ../src/obu -name Makefile); 
do 
	# Change to the folder containing Makefile
	cd $(dirname $i); 
	echo -n "Building $(basename $PWD) . . . "

	# Clean the folder and build for linux.
	make clean &> /dev/null
	make linux &> /dev/null

	# Indicate Success/Failure of the build
	if [ "$?" -eq "0" ]; then
		echo -e "${green}Success${nocolor}"

		# If succeeded, then copy the executable to the applications folder
		for j in $(find -maxdepth 1); do
			# for all the files, check if it is a regular file first, and then,
			# check if it is an executable file. Then copy. Then clean the folder.
			if [ -f $j ] && [ -x $j ]; then
				cp $j $appdir/
				make clean &> /dev/null
			fi
		done
	else
		echo -e "${red}Failure${nocolor}"
	fi

	# Clean the folder before leaving to keep it clean for svn and/or other stuff
	make clean &> /dev/null

	# Return back to original directory to go over the process again for another one
	cd - &> /dev/null
done
