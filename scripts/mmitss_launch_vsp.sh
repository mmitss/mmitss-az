read -p "Full absolute path of MMITSS configuration directory: " config_path
read -p "Name of container image on the Dockerhub : " container_image

docker run -v $config_path:/nojournal --network host --name vsp_container $container_image > /dev/null 2>&1 &
