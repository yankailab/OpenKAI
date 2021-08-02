#!/bin/bash

# Enable SSH, disable OS use of UART, enable UART hardware
sudo raspi-config

# Setup
sudo apt-get update
sudo apt-get upgrade
sudo apt-get dist-upgrade
sudo rpi-update

# Disable OS use of UART and Enable UART hardware
set +H
sudo sh -c "echo 'dtoverlay=disable-bt\ndtoverlay=disable-wifi\n' >> /boot/config.txt"
set -H
sudo reboot now

sudo systemctl disable hciuart
sudo systemctl stop serial-getty@ttyAMA0.service
sudo systemctl disable serial-getty@ttyAMA0.service
sudo systemctl stop serial-getty@ttyS0.service
sudo systemctl disable serial-getty@ttyS0.service
sudo reboot now
	
# Raspberry camera
sudo apt-get -y install gstreamer1.0-omx libraspberrypi-dev
git clone https://github.com/thaytan/gst-rpicamsrc.git
cd gst-rpicamsrc
./autogen.sh --prefix=/usr --libdir=/usr/lib/arm-linux-gnueabihf/
make
sudo make install

# Wifi Setup
# Disable the internal Wifi
cat << EOF | sudo tee -a /etc/modprobe.d/raspi-blacklist.conf
#wifi
blacklist brcmfmac
blacklist brcmutil
EOF
sudo reboot now

#Plug USB wifi and run
sudo sh -c 'wpa_passphrase (ssid) (passphrase) | grep -v "#psk=" >> /etc/wpa_supplicant/wpa_supplicant.conf'
sudo reboot now

# Auto start on bootup
sudo rm /etc/rc.local
set +H
sudo sh -c "echo '#!/bin/sh\n/home/ubuntu/jetson_clocks.sh\nnvpmodel -m 0\nexit 0\n' >> /etc/rc.local"
set -H
sudo chmod a+x /etc/rc.local
