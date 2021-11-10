#!/bin/bash

sudo apt-get -y update
sudo apt-get -y install nano

# Rotate screen
# DISPLAY=:0 xrandr -o right
echo -e '#!/bin/sh\nDISPLAY=:0 xrandr -o right\n' > ~/xrandr.sh
sudo chmod a+x ~/xrandr.sh
# add xrandr.sh to Startup Application

# Rotate touchscreen
# for Ubuntu and Jetson
sudo apt install xserver-xorg-input-synaptics
# for Raspberry Pi
# sudo apt-get install xserver-xorg-input-libinput
# create the following dir if there isn't one
sudo mkdir /etc/X11/xorg.conf.d
sudo cp /usr/share/X11/xorg.conf.d/40-libinput.conf /etc/X11/xorg.conf.d/
sudo nano /etc/X11/xorg.conf.d/40-libinput.conf 
# add the following line into "touchscreen" section, then save it
Option "CalibrationMatrix" "0 1 0 -1 0 1 0 0 1"
# rotation matrix
90 ： Option "CalibrationMatrix" "0 1 0 -1 0 1 0 0 1"
180： Option "CalibrationMatrix" "-1 0 1 0 -1 1 0 0 1"
270： Option "CalibrationMatrix" "0 -1 1 1 0 0 0 0 1"

# Change performace setting and make it auto start
sudo rm /etc/rc.local
set +H
sudo sh -c "echo '#!/bin/sh\njetson_clocks\nnvpmodel -m 8\n/home/lab/ok.sh &\nexit 0\n' >> /etc/rc.local"
set -H
sudo chmod a+x /etc/rc.local
#sudo nvpmodel -q --verbose
#set nvpmodel -m 0 for Jetson Nano
#set nvpmodel -m 8 for Jetson Xavier NX

# (Optional) auto mount sd card on boot
#sudo sh -c "echo '#!/bin/sh\njetson_clocks\nnvpmodel -m 0\nmount /dev/mmcblk1p1 /mnt/sd\n/home/lab/ok.sh &\nexit 0\n' >> /etc/rc.local"

# Switching start up desktop
sudo systemctl set-default multi-user.target
#sudo systemctl set-default graphical.target

# Delete unused modules that conflicts the serial comm
sudo systemctl stop ModemManager
sudo apt-get -y purge whoopsie modemmanager
sudo systemctl stop nvgetty
sudo systemctl disable nvgetty
udevadm trigger
sudo apt -y remove --purge libreoffice* yelp thunderbird rhythmbox

sudo apt autoremove
sudo reboot now

# Migrate system to SD card / NVMe if necessary

# Clone SD image
sudo dd if=/dev/sdb of=~/Jetson.img

# set CSI camera interface
# if crashed run it from eMMC boot then copy the dtb file to SD card
# /boot/kernel_tegra194-p3668-all-p3509-0000-user-custom.dtb.
sudo /opt/nvidia/jetson-io/jetson-io.py

# install CUDA etc. with Nvidia SDK Manager

# USB reset for Realsense
sudo apt-get install libusb-1.0-0-dev
git clone --depth 1 https://github.com/mvp/uhubctl.git
cd uhubctl
make
sudo make install
sudo uhubctl -a cycle -l 1 -p 1-4
sudo uhubctl -a cycle -l 2 -p 1-4

#Plug USB wifi and run
sudo sh -c 'wpa_passphrase (ssid) (passphrase) | grep -v "#psk=" >> /etc/wpa_supplicant/wpa_supplicant.conf'
sudo reboot now

# Auto start on bootup
sudo rm /etc/rc.local
set +H
sudo sh -c "echo '#!/bin/sh\n/home/pi/usbReset.sh\n/home/pi/ok.sh &\nexit 0\n' >> /etc/rc.local"
set -H
sudo chmod a+x /etc/rc.local

# stop services
sudo systemctl disable apt-daily.service
sudo systemctl disable apt-daily.timer
sudo systemctl disable apt-daily-upgrade.timer
sudo systemctl disable apt-daily-upgrade.service
sudo systemctl disable man-db.service
sudo systemctl disable wpa_supplicant.service
sudo systemctl disable wifi-country.service
sudo systemctl disable avahi-daemon.service
sudo systemctl disable rsyslog.servic
sudo systemctl disable hciuart.service
sudo systemctl disable triggerhappy.service

sudo systemctl disable keyboard-setup.service
sudo systemctl disable ntp.service
sudo systemctl disable dhcpcd.service
sudo systemctl disable networking.service
sudo systemctl disable ssh.service

# show no cursor

# show no taskbar

# auto start GUI app with systemd
# disable ncurses (Console)
sudo nano /lib/systemd/system/ok.service
sudo systemctl daemon-reload
sudo systemctl enable ok.service

# clear bash history
history -c
