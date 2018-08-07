Instructions
==

* Install docker.io and lxc packages
* Create a symlink called docker in your path linking to the docker.io binary (if necessary)
* Run this command: 
~~~
echo "DOCKER_OPTS=\"-e lxc\"" >> /etc/defaults/docker.io
~~~
* Add the current user to the docker group (to avoid using sudo)
* First run the build.sh script. This is required only once per version.
* Populate the "intersections" folder with configurations of the respective intersections
* Run the following command to spawn the containers
~~~
./run.sh all
~~~
