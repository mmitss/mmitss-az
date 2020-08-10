read -p "Full absolute path of MMITSS configuration directory: " config_path
read -p "Name of container image on the Dockerhub : " container_image
read -p "Name of container: " container_name

docker run -v $config_path:/nojournal --network host --name $container_name $container_image > /dev/null 2>&1 &
