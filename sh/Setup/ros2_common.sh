#----------------------------------------------------
# (Optional) ros2
# https://docs.ros.org/en/humble/Installation/Alternatives/Ubuntu-Development-Setup.html

# check for UTF-8
locale

sudo apt update && sudo apt install locales
sudo locale-gen en_US en_US.UTF-8
sudo update-locale LC_ALL=en_US.UTF-8 LANG=en_US.UTF-8
sudo echo -e "export LC_ALL=en_US.UTF-8" >> ~/.bashrc

# verify settings
locale

# add the ROS 2 apt repository
sudo apt install software-properties-common
sudo add-apt-repository universe

# add the ROS 2 GPG key with apt.
sudo apt update && sudo apt install curl -y
sudo curl -sSL https://raw.githubusercontent.com/ros/rosdistro/master/ros.key -o /usr/share/keyrings/ros-archive-keyring.gpg

# add the repository to your sources list.
echo "deb [arch=$(dpkg --print-architecture) signed-by=/usr/share/keyrings/ros-archive-keyring.gpg] http://packages.ros.org/ros2/ubuntu $(. /etc/os-release && echo $UBUNTU_CODENAME) main" | sudo tee /etc/apt/sources.list.d/ros2.list > /dev/null

# install common packages
sudo apt update && sudo apt install -y \
  python3-flake8-docstrings \
  python3-pip \
  python3-pytest-cov \
  ros-dev-tools

# install packages for Ubuntu 22.04
sudo apt install -y \
   python3-flake8-blind-except \
   python3-flake8-builtins \
   python3-flake8-class-newline \
   python3-flake8-comprehensions \
   python3-flake8-deprecated \
   python3-flake8-import-order \
   python3-flake8-quotes \
   python3-pytest-repeat \
   python3-pytest-rerunfailures

# get ROS 2 code
# create a workspace and clone all repos:
mkdir -p ~/dev/ros2_humble/src
cd ~/dev/ros2_humble
vcs import --input https://raw.githubusercontent.com/ros2/ros2/humble/ros2.repos src

# install dependencies using rosdep
sudo apt upgrade
sudo rosdep init
# sudo rm -rf /etc/ros/rosdep/sources.list.d/20-default.list
rosdep update
rosdep install --from-paths src --ignore-src -y --skip-keys "fastcdr rti-connext-dds-6.0.1 urdfdom_headers"

# build the code in the workspace
# if you have already installed ROS 2 another way (either via Debians or the binary distribution), make sure that you run the below commands in a fresh environment that does not have those other installations sourced. Also ensure that you do not have source /opt/ros/${ROS_DISTRO}/setup.bash in your .bashrc. You can make sure that ROS 2 is not sourced with the command printenv | grep -i ROS. The output should be empty.
# more info on working with a ROS workspace can be found in
# https://docs.ros.org/en/humble/Tutorials/Beginner-Client-Libraries/Colcon-Tutorial.html
cd ~/dev/ros2_humble/

sudo apt-get install -y libtinyxml2-dev libasio-dev
sudo pip install numpy

#colcon build --symlink-install
colcon build --symlink-install --packages-ignore qt_gui_app rviz_rendering rviz_rendering_tests qt_gui_cpp rqt_gui_cpp turtlesim rviz_common rviz_visual_testing_framework rviz_default_plugins rviz2
# rosidl_generator_py rclpy

# if you are having trouble compiling all examples and this is preventing you from completing a successful build,
# you can use COLCON_IGNORE in the same manner as CATKIN_IGNORE to ignore the subtree or remove the folder from the workspace.
# Take for instance: you would like to avoid installing the large OpenCV library.
# Well then simply run touch COLCON_IGNORE in the cam2image demo directory to leave it out of the build process.

# environment setup
# source the setup script
# set up your environment by sourcing the following file
. ~/dev/ros2_humble/install/local_setup.bash

# try some examples
# in one terminal, source the setup file and then run a C++ talker:
. ~/dev/ros2_humble/install/local_setup.bash
ros2 run demo_nodes_cpp talker

# in another terminal source the setup file and then run a Python listener:
. ~/dev/ros2_humble/install/local_setup.bash
ros2 run demo_nodes_py listener

# you should see the talker saying that it’s Publishing messages and the listener saying I heard those messages. This verifies both the C++ and Python APIs are working properly. Hooray!

# ros package build
mkdir -p ~/dev/rosWS/src
cd ~/dev/rosWS/src
ros2 pkg create --build-type ament_cmake ok_ros

cd ~/dev/rosWS && colcon build
