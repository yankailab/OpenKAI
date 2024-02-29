#----------------------------------------------------
# (Optional) Fast_lio ros2
sudo apt-get install libpcl-dev

# install Livox-SDK2 following deps_common.sh

# install ros2 following the ros2_common.sh

# inside ros workspace/src
mkdir -p ~/dev/rosWS/src
cd ~/dev/rosWS/src
git clone https://github.com/Livox-SDK/livox_ros_driver2.git
cd livox_ros_driver2
./build.sh humble

echo "source /home/kai/dev/rosWS/install/setup.bash" >> ~/.bashrc

# build Fast_lio
cd ~/dev/rosWS/src
git clone https://github.com/Ericsii/FAST_LIO_ROS2.git --recursive
cd ..
rosdep install --from-paths src --ignore-src -y
colcon build --symlink-install
./install/setup.bash # use setup.zsh if use zsh

# edit livox config
nano ~/dev/rosWS/install/livox_ros_driver2/share/livox_ros_driver2/config/MID360_config.json

# verify the point cloud output
ros2 launch livox_ros_driver2 rviz_MID360_launch.py

# launch msg for fast_lio
ros2 launch livox_ros_driver2 msg_MID360_launch.py

nano ~/dev/rosWS/install/fast_lio/share/fast_lio/config/mid360.yaml
ros2 launch fast_lio mapping.launch.py config_file:=mid360.yaml


#----------------------------------------------------
# Fast_lio ros2 on docker

#docker rm -f $(docker ps -a -q)

sudo apt-get install docker.io
sudo chmod 666 /var/run/docker.sock 

# nvidia-docker
curl -fsSL https://nvidia.github.io/libnvidia-container/gpgkey | sudo gpg --dearmor -o /usr/share/keyrings/nvidia-container-toolkit-keyring.gpg \
  && curl -s -L https://nvidia.github.io/libnvidia-container/stable/deb/nvidia-container-toolkit.list | \
    sed 's#deb https://#deb [signed-by=/usr/share/keyrings/nvidia-container-toolkit-keyring.gpg] https://#g' | \
    sudo tee /etc/apt/sources.list.d/nvidia-container-toolkit.list

sudo apt-get update
sudo apt-get install -y nvidia-container-toolkit
sudo nvidia-ctk runtime configure --runtime=docker
sudo systemctl restart docker

# fast lio ros2
git clone --recursive https://github.com/Taeyoung96/FAST_LIO_ROS2.git
cd FAST_LIO_ROS2/docker/
docker build -t fast-lio-ros2:latest .

# replace in container_run.sh
docker run --gpus all --privileged -it \
 -e NVIDIA_DRIVER_CAPABILITIES=all \
 -e NVIDIA_VISIBLE_DEVICES=all \
 --volume="$PROJECT_DIR:/root/ros2_ws/src" \
 --volume=/data/LIDAR_dataset:/root/data \
 --volume=/tmp/.X11-unix:/tmp/.X11-unix:rw \
 --net=host \
 --ipc=host \
 --shm-size=4gb \
 --name="$CONTAINER_NAME" \
 --env="DISPLAY=$DISPLAY" \
 "$IMAGE_NAME" /bin/bash

# run the docker container
xhost +local:docker
 ./container_run.sh fast-lio-ros2 fast-lio-ros2:latest

# build FAST_LIO
sudo apt-get install nano
nano src/livox_ros_driver2 /build.sh
-    colcon build --cmake-args -DROS_EDITION=${VERSION_ROS2} -DHUMBLE_ROS=${ROS_HUMBLE}
+    colcon build  --symlink-install --cmake-args -DROS_EDITION=${VERSION_ROS2} -DHUMBLE_ROS=${ROS_HUMBLE}

# build Livox ros2 driver
cd ~/ros2_ws/src/livox_ros_driver2
./build.sh humble

# configure the host and Mid360 ip
nano livox_ros_driver2/config/MID360_config.json

# restart container
docker container restart fast-lio-ros2

# run livox_ros_driver2
xhost +local:docker
docker exec -it fast-lio-ros2 /bin/bash
source ~/ros2_ws/install/setup.bash
ros2 launch livox_ros_driver2 rviz_MID360_launch.py 

# run fast_lio on another terminal
xhost +local:docker
docker exec -it fast-lio-ros2 /bin/bash
source ~/ros2_ws/install/setup.bash
ros2 launch fast_lio mapping_mid360.launch.py 