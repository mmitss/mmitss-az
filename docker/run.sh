#!/bin/bash

GATEWAY="10.254.56.53"
repo_name="mmitssuofa"
img_name="rse"
tag_name="latest"
docker="docker.io"
#docker="docker -D"

# Function to check if a container is already running and spawn a new container
# IMPORTANT: the variable $IP must be set before calling this function
run_container () {
	# Some preliminary checks
	[ -z "$IP" ] && return
	container_name=device_$IP

	# check if a container with the same name is already running
	$docker ps -a | grep $container_name &> /dev/null
	if [ "$?" -eq "0" ]; then
		echo "$container_name: Container already exists. Delete it with \"$docker rm <container_name>\" first"
		return
	fi
	
	# Create a new container here. Options explained inline
	echo -n "Creating a new Container with IP = $IP : "

	# Setup debug mode entry. Define an env variable called MMITSS_DOCKER_DEBUG with any value to enable debugging
	if [ -z "$MMITSS_DOCKER_DEBUG" ]; then
		DOCKER_RUN_OPTS="-d"
		DOCKER_RUN_CMD="/bin/bash -c /mmitss/bootstrap.sh"
	else
		DOCKER_RUN_OPTS="-it"
		DOCKER_RUN_CMD="/bin/bash"
	fi

	# Command to run the conatiner as a daemon if -d is used or as an interactive pseudo-terminal if -it is used
	$docker run $DOCKER_RUN_OPTS  \
		`### do not assume any network configuration` \
		--net="none" \
		\
		`### LXC options follow` \
		`### LXC provides a virtual ethernet interface to the host` \
		--lxc-conf="lxc.network.type=veth" \
		`### The Static IP address that the container should use` \
		--lxc-conf="lxc.network.ipv4=$IP/24" \
		`### The host machine will be the gateway to the outer internet` \
		--lxc-conf="lxc.network.ipv4.gateway=$GATEWAY" \
		`### A pre-configured bridged interface called docker999 is used as a bridge to eth0/eth1` \
		--lxc-conf="lxc.network.link=docker999" \
		`### This interface will show up as eth0 inside the container` \
		--lxc-conf="lxc.network.name=p1p1" \
		`### Up the interface` \
		--lxc-conf="lxc.network.flags=up" \
		\
		`### regular docker options follow` \
		`### Mount a preconfigured folder as nojournal` \
		-v $config_dir/nojournal:/nojournal \
		`### Mount folder containing applications to be used inside the container` \
		-v $PWD/applications:/mmitss \
		\
		`### set the hostname of the container. hostname=IP_Addr|dots replaced by underscores` \
		-h "${IP//./_}" \
		`### set the name of the container to show up in docker ps` \
		--name "$container_name" \
		`### Specify the repo/image:tag to use to spawn the container` \
		$repo_name/$img_name:$tag_name \
		`### Specify the binary to load when the container loads` \
		$DOCKER_RUN_CMD
}

run_all () {
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

		run_container
	done

	# set +x
}

if [ $# -lt 1 ]; then
	echo "Usage: $0 <list_of_config_dirs>"
	exit 1
else
	# check if the required image is present
	$docker images $repo_name/$img_name | grep $tag_name &> /dev/null
	if [ ! "$?" -eq "0" ]; then
		echo "Pull/Build the RSE image by running \"./build.sh\" before continuing"
		exit 1
	fi
	run_all $@
fi
