#----------------------------------------------------
# Clean-up
sudo apt -y remove --purge libreoffice* yelp thunderbird rhythmbox
sudo systemctl stop ModemManager
sudo systemctl disable ModemManager
sudo apt-get -y purge whoopsie modemmanager

sudo systemctl disable apt-daily.service
sudo systemctl disable apt-daily.timer
sudo systemctl disable apt-daily-upgrade.timer
sudo systemctl disable apt-daily-upgrade.service
sudo systemctl disable man-db.service

sudo systemctl disable hciuart.service
sudo systemctl disable triggerhappy.service
sudo systemctl disable keyboard-setup.service

sudo systemctl disable wpa_supplicant.service
sudo systemctl disable wifi-country.service
sudo systemctl disable avahi-daemon.service
sudo systemctl disable ntp.service
sudo systemctl disable dhcpcd.service
sudo systemctl disable networking.service
sudo systemctl disable ssh.service

# clear system logs
sudo systemctl stop rsyslog
sudo systemctl disable rsyslog.service
sudo systemctl stop systemd-journald.service
sudo systemctl disable systemd-journald.service
sudo systemctl mask systemd-journald.service
sudo truncate /var/log/*.log --size 0

# clear bash history
history -c && history -w

# ubuntu power mode
cat /sys/devices/system/cpu/cpu0/cpufreq/scaling_available_governors
cat /sys/devices/system/cpu/cpu0/cpufreq/scaling_governor
echo performance | sudo tee /sys/devices/system/cpu/cpu*/cpufreq/scaling_governor

# add user to dialout
sudo adduser [username] dialout
#sudo gpasswd --add ${USER} dialout
sudo reboot now
#sudo chmod 666 /dev/ttyUSB0

#----------------------------------------------------
# Config

# When using multiple USB-Serial converter
sudo apt remove brltty

#----------------------------------------------------
# Auto startup

# Copy startup sh into home
sudo chmod a+x $HOME/ok.sh

# For startup on X, insert sleep before running OK to wait for X to be ready
sleep 5
./OK [kiss]

# Auto start up on boot
sudo rm /etc/rc.local
set +H
sudo sh -c "echo '#!/bin/sh\n/home/lab/ok.sh\nexit 0\n' >> /etc/rc.local"
set -H
sudo chmod a+x /etc/rc.local

#----------------------------------------------------
# Misc.

# GDB
gdb --args executablename arg1 arg2 arg3
run
bt

# Upload to FTP
curl -T FourierToy.swf ftp://193.112.75.123/pub/ --user anonymous

# Test gstreamer
sudo gst-launch-1.0 v4l2src device=/dev/video0 ! video/x-raw,width=640,height=480,framerate=20/1 ! x264enc ! matroskamux ! filesink location=/home/pi/ssd/test.mkv
sudo gst-launch-1.0 v4l2src device=/dev/video0 ! video/x-raw,width=1280,height=720,framerate=30/1 ! videoconvert ! fbdevsink

#----------------------------------------------------
# Screen

# Disable wayland in Ubuntu 22.04
sudo nano /etc/gdm3/custom.conf
#WaylandEnable=false

# Screen and touch screen input rotate
export DISPLAY=:0.0
xrandr -o left
xinput list
xinput set-prop 'GXTP7386:00 27C6:0113' 'Coordinate Transformation Matrix' 0 -1 1 1 0 0 0 0 1

# Block unity touch gestures
sudo apt install dconf-editor
dconf-editor
com > canonical > unity > gestures

# Block on screen keyboard
sudo apt install gnome-tweaks
sudo apt install gnome-shell-extensions
https://extensions.gnome.org/extension/3222/block-caribou-36/

#----------------------------------------------------
# Make Jetson boot SD image

sudo fdisk -l
sudo umount /dev/sda
sudo dd if=/dev/sda of=~/sd.img bs=6M

sudo fdisk -l
sudo umount /dev/sdb
sudo dd if=~/sd.img of=/dev/sdb bs=6M

#----------------------------------------------------
# Sunlogin auto startup
Statup applications
/usr/local/sunlogin/bin/sunloginclient --cmd=autorun
# Disable wayland
sudo nano /etc/gdm3/custom.conf
#WaylandEnable=false


#----------------------------------------------------
# serial test
sudo apt-get install cutecom
