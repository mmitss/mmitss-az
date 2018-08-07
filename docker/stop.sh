#!/bin/bash
# script to stop all running docker containers

# docker rm -f $(docker ps -aq)
stop_all () {
	# set -x

	for config_dir in $@; do
		cd $config_dir
		config_dir=$PWD
		cd - &> /dev/null

		# check if the configuration directory exists
		if [ ! -d $config_dir ] || [ ! -e "$config_dir/config" ]; then 
			echo "ERROR: Config directory doesn't exist at $config_dir"
			continue
		fi
		source $config_dir/config

		container_name=device_$IP
		echo -n "Stopping container: "
		#docker.io rm -f $container_name
		docker rm -f $container_name

	done

	# set +x
}

if [ $# -lt 1 ]; then
	echo "Usage: $0 <list_of_config_dirs>"
	exit 1
else
	stop_all $@
fi
