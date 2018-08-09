#!/bin/bash
## script to setup context for docker and build an image

libj2735=libj2735-linux.a 
repo_name=mmitssuofa/rse
tag_name=latest
#docker="docker.io"
docker="docker"


select choice in "Build form Dockerfile" "Pull from Docker Hub"; do
	case $REPLY in
		1 )
			# If libj2735 is available in suitable path or in the PWD, then build from Dockerfile
			echo "Building from Dockerfile. May take a long time if this is the first time you are building it"
			sleep 3s
			if [ -e $libj2735 ]; then cp -v $libj2735 .; 
			elif [ ! -e $(basename $libj2735) ]; then echo "libj2735-linux.a not found! Aborting build."; exit -1; fi
			$docker build -t $repo_name:$tag_name .
			break
			;;
		2 )
			# Else, download directly from docker Hub
			echo "Pulling image from docker hub. May take a long time depending on the internet connection available"
			$docker pull $repo_name:$tag_name
			break
			;;
	esac
done
