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
git clone --depth 1 https://github.com/thaytan/gst-rpicamsrc.git
cd gst-rpicamsrc
./autogen.sh --prefix=/usr --libdir=/usr/lib/arm-linux-gnueabihf/
make
sudo make install

# USB reset for Realsense
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

#Plug USB wifi and run
sudo sh -c 'wpa_passphrase (ssid) (passphrase) | grep -v "#psk=" >> /etc/wpa_supplicant/wpa_supplicant.conf'
sudo reboot now

# Auto start on bootup
sudo rm /etc/rc.local
set +H
sudo sh -c "echo '#!/bin/sh\n/home/pi/usbReset.sh\n/home/pi/ok.sh &\nexit 0\n' >> /etc/rc.local"
set -H
sudo chmod a+x /etc/rc.local



# speed up

# overlays
sudo bash -c 'printf "enable_uart=1\ndisable_splash=1\ndtoverlay=disable-bt\ndtoverlay=disable-wifi\nboot_delay=0\n" >> /boot/config.txt'
# optional
#force_turbo=1
#dtoverlay=sdtweak,overclock_50=100

# remove the msg under splash image
sudo vi /usr/share/plymouth/themes/pix/pix.script
remove (or comment out) four lines below:
message_sprite = Sprite();
message_sprite.SetPosition(screen_width * 0.1, screen_height * 0.9, 10000);

       my_image = Image.Text(text, 1, 1, 1);
       message_sprite.SetImage(my_image);

# boot command line
sudo vi /bool/cmdline.txt
console=tty1 root=PARTUUID=0157fd94-02 rootfstype=ext4 elevator=deadline fsck.repair=yes rootwait quiet splash plymouth.ignore-serial-consoles fastboot

# replace splash image
sudo cp ~/my_splash.png /usr/share/plymouth/themes/pix/splash.png

# stop services
sudo systemctl disable apt-daily.service
sudo systemctl disable apt-daily.timer
sudo systemctl disable apt-daily-upgrade.timer
sudo systemctl disable apt-daily-upgrade.service
sudo systemctl disable man-db.service
sudo systemctl disable rpi-eeprom-update.service
sudo systemctl disable wpa_supplicant.service
sudo systemctl disable wifi-country.service
sudo systemctl disable avahi-daemon.service
sudo systemctl disable rsyslog.servic
sudo systemctl disable hciuart.service
sudo systemctl disable triggerhappy.service

sudo systemctl disable raspi-config.service
sudo systemctl disable keyboard-setup.service
sudo systemctl disable ntp.service
sudo systemctl disable dhcpcd.service
sudo systemctl disable networking.service
sudo systemctl disable ssh.service

# show no cursor
sudo nano /etc/lightdm/lightdm.conf
xserver-command=X -nocursor

# show no taskbar
sudo nano /etc/xdg/lxsession/LXDE-pi/autostart
#@lxpanel --profile LXDE-pi

# auto start GUI app with systemd
# disable ncurses (Console)
sudo nano /lib/systemd/system/ok.service
sudo systemctl daemon-reload
sudo systemctl enable ok.service
