#!/bin/bash

# Disable wayland in Ubuntu 22.04
sudo nano /etc/gdm3/custom/config
#WaylandEnable=false

# Screen and touch screen input rotate
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

# Delete unused modules
sudo apt -y remove --purge libreoffice* yelp thunderbird rhythmbox
sudo apt autoremove
sudo reboot now

# Clone SD image
sudo dd if=/dev/sdb of=~/Jetson.img

# Auto start on bootup
#!/bin/bash
/home/lab/dev/OpenKAI/build/OpenKAI /home/lab/VzScan/VzScan.kiss

# stop services
sudo systemctl disable apt-daily.service
sudo systemctl disable apt-daily.timer
sudo systemctl disable apt-daily-upgrade.timer
sudo systemctl disable apt-daily-upgrade.service
sudo systemctl disable man-db.service
sudo systemctl disable rsyslog.servic
sudo systemctl disable hciuart.service
sudo systemctl disable triggerhappy.service

# show no cursor

# show no taskbar

# clear bash history
history -c
