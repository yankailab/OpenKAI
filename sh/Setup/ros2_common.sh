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

# add the ROS 2 apt repository
sudo apt install software-properties-common
sudo add-apt-repository universe

# add the ROS 2 GPG key with apt.
sudo apt update && sudo apt install curl -y
sudo curl -sSL https://raw.githubusercontent.com/ros/rosdistro/master/ros.key -o /usr/share/keyrings/ros-archive-keyring.gpg

# add the repository to your sources list.
echo "deb [arch=$(dpkg --print-architecture) signed-by=/usr/share/keyrings/ros-archive-keyring.gpg] http://packages.ros.org/ros2/ubuntu $(. /etc/os-release && echo $UBUNTU_CODENAME) main" | sudo tee /etc/apt/sources.list.d/ros2.list > /dev/null
#echo "deb [arch=$(dpkg --print-architecture) signed-by=/usr/share/keyrings/ros-archive-keyring.gpg] http://packages.ros.org/ros2/ubuntu bookworm main" | sudo tee /etc/apt/sources.list.d/ros2.list > /dev/null

# install common packages
sudo apt update
sudo apt upgrade

# Install prebuilt binary -----------------
sudo apt install ros-humble-desktop
#sudo apt install ros-humble-ros-base
sudo apt install ros-dev-tools

source /opt/ros/humble/setup.bash
#echo "source /opt/ros/humble/setup.bash" >> ~/.bashrc

sudo apt upgrade
sudo rosdep init
rosdep update


# Build from source for Iron or on Raspberry Pi OS -----------------
sudo apt install -y \
  python3-flake8-docstrings \
  python3-pip \
  python3-pytest-cov \
  ros-dev-tools

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
mkdir -p ~/dev/ros2_iron/src
cd ~/dev/ros2_iron
vcs import --input https://raw.githubusercontent.com/ros2/ros2/iron/ros2.repos src

# install dependencies using rosdep
sudo apt upgrade
#sudo rm -rf /etc/ros/rosdep/sources.list.d/20-default.list
sudo rosdep init
rosdep update
rosdep install --from-paths src --ignore-src -y --skip-keys "fastcdr rti-connext-dds-6.0.1 urdfdom_headers"

colcon build --symlink-install
# if qtbase5-dev unmet, using aptitude to solve dependencies
sudo apt-get install aptitude
sudo aptitude install qtbase5-dev

# For Raspberry Pi
#sudo apt-get install screen
export MAKEFLAGS="-j2"
# add --cmake-args -DCMAKE_BUILD_TYPE=Release for Release build
#screen colcon build --symlink-install --parallel-workers 1 --packages-ignore qt_gui_app rviz_rendering rviz_rendering_tests qt_gui_cpp rqt_gui_cpp turtlesim rviz_common rviz_visual_testing_framework rviz_default_plugins rviz2 rosbag2_transport
screen colcon build --cmake-clean-cache --symlink-install --parallel-workers 1 --packages-ignore-regex '.*qt' '.*rviz' '.*rqt' '.*turtlesim' '.*example' '.*demo' '.*test' '.*rosbag' --cmake-args -DCMAKE_BUILD_TYPE=Release
#Ctrl+A Ctrl+D
#screen -ls
#screen -r xxxx

# source the setup script
echo "source /home/lab/dev/ros2_iron/install/local_setup.bash" >> ~/.bashrc

# try some examples
ros2 run demo_nodes_cpp talker
ros2 run demo_nodes_cpp listener

# ros package build
mkdir -p ~/dev/rosWS/src
cd ~/dev/rosWS/src
ros2 pkg create --build-type ament_cmake ok_ros
cd ~/dev/rosWS && colcon build
