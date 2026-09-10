#!/bin/bash

# Enable SSH, disable OS use of UART, enable UART hardware
# Expanding root partition
sudo raspi-config
-> Advanced Options
-> Expand Filesystem

# Setup
sudo apt-get update
sudo apt-get upgrade
sudo apt-get dist-upgrade
# sudo rpi-update

--------------
# Connect to wifi
# https://www.raspberrypi.com/documentation/computers/configuration.html
sudo nmcli dev wifi connect <example_ssid>

# Assign static IP to eth0
# using nmcli
sudo nmcli -p connection show
sudo nmcli con mod "netplan-eth0" ipv4.addresses 192.168.16.243/24 ipv4.gateway 192.168.16.1 ipv4.dns 192.168.16.1 ipv4.method manual
sudo nmcli c down "netplan-eth0" && sudo nmcli c up "netplan-eth0"

start.sh--
#!/bin/bash
source /home/lab/dev/ros2_humble/install/local_setup.bash
source /home/lab/dev/rosWS/install/setup.bash

/home/lab/lv.sh
#ros2 launch livox_ros_driver2 msg_MID360_launch.py &
sleep 5
/home/lab/fl.sh
#ros2 launch fast_lio mapping.launch.py config_file:=mid360.yaml &
sleep 5
/home/lab/ok.sh
#/home/lab/dev/OpenKAI/build/OpenKAI /home/lab/dev/OpenKAI/kiss/app/apCopter_fastLio.kiss &

exit 0
--





# To be updated

--------------
# Disable OS use of UART and Enable UART hardware
set +H
sudo sh -c "echo 'dtoverlay=disable-bt\n' >> /boot/firmware/config.txt"
set -H
sudo reboot now
#sudo sh -c "echo 'dtoverlay=disable-bt\ndtoverlay=disable-wifi\n' >> /boot/firmware/config.txt"

# Enable multiple UART
dtoverlay=uart3        # without flow control pins
dtoverlay=uart3,ctsrts # with flow control pins


--------------
# Raspberry camera
sudo nano /boot/firmware/config.txt
#camera_auto_detect=1
dtoverlay=imx219
#dtoverlay=imx219,cam0
# old ref: https://www.waveshare.net/wiki/CM4-NANO-A

# verify
libcamera-hello

sudo apt install gstreamer1.0-libcamera
gst-launch-1.0 libcamerasrc ! video/x-raw,width=1280,height=720,framerate=30/1 ! videoconvert ! videoflip method=0 ! queue ! videoconvert ! v4l2h264enc ! 'video/x-h264,level=(string)5' ! h264parse ! rtph264pay mtu=1400 config-interval=1 pt=96 ! udpsink host=192.168.1.192 port=5678 auto-multicast=false

gst-launch-1.0 v4l2src device=/dev/video0 ! video/x-raw,width=640,height=480,framerate=20/1 ! x264enc ! matroskamux ! filesink location=/home/pi/ssd/test.mkv
gst-launch-1.0 v4l2src device=/dev/video0 ! video/x-raw,width=1280,height=720,framerate=30/1 ! v4l2h264enc ! h264parse ! matroskamux ! filesink location=/home/pi/ssd/test1.mkv
gst-launch-1.0 v4l2src device=/dev/video0 ! video/x-raw,width=1280,height=720,framerate=30/1 ! videoconvert ! fbdevsink
gst-launch-1.0 libcamerasrc ! video/x-raw,width=1280,height=720,framerate=30/1 ! videoconvert ! fbdevsink
gst-launch-1.0 libcamerasrc ! video/x-raw,format=RGB,width=1280,height=720,framerate=30/1 ! v4l2convert ! v4l2h264enc ! 'video/x-h264,level=(string)4' ! h264parse ! matroskamux ! filesink location="/home/pi/ssd/video/test.mka"
gst-launch-1.0 libcamerasrc ! video/x-raw,format=RGB,width=1280,height=720,framerate=30/1 ! v4l2convert ! v4l2h264enc ! 'video/x-h264,level=(string)4' ! h264parse ! matroskamux ! filesink location="/home/pi/ssd/video/test.mka"


--------------
# Enable NvMe SSD
sudo nano /boot/firmware/config.txt
dtparam=nvme

# Optional, if needed to enable rrprom update on CM4
Add these the following entries to /etc/default/rpi-eeprom-update
RPI_EEPROM_USE_FLASHROM=1
CM4_ENABLE_RPI_EEPROM_UPDATE=1
and these entries to config.txt and reboot
[cm4]
dtparam=spi=on
dtoverlay=audremap
dtoverlay=spi-gpio40-45


# Mount ssd
lsblk -f
sudo mkfs -t ext4 /dev/nvme0n1
# sudo mkfs -t vfat /dev/nvme0n1
# use ntfs for external usb memory applications
sudo mkdir -p /home/pi/ssd
sudo mount -t auto /dev/nvme0n1 /home/pi/ssd
sudo chown pi ssd/
sudo mount /dev/nvme0n1 /home/pi/ssd




# USB reset for Realsense
sudo nano /boot/config.txt
-----------------------
#otg_mode=1
dtoverlay=dwc2,dr_mode=host
-----------------------

sudo apt-get install libusb-1.0-0-dev
git clone --depth 1 https://github.com/mvp/uhubctl.git
cd uhubctl
make
sudo make install
sudo uhubctl -a cycle -l 1 -p 1-4
sudo uhubctl -a cycle -l 2 -p 1-4

# Wifi Setup
# Disable the internal Wifi
cat << EOF | sudo tee -a /etc/modprobe.d/raspi-blacklist.conf
#wifi
blacklist brcmfmac
blacklist brcmutil
EOF
sudo reboot now

# CM4 switch to external antenna.
sudo nano /boot/config.txt
-----------------------
dtparam=ant2
Params:
ant1                    Select antenna 1 (default). CM4 only.
ant2                    Select antenna 2. CM4 only.
noant                   Disable both antennas. CM4 only.
-----------------------

