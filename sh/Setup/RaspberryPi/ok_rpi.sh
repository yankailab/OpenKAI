#!/bin/bash

# Enable SSH, disable OS use of UART, enable UART hardware
sudo raspi-config

Start the SSH service with systemctl
sudo systemctl enable ssh
sudo systemctl start ssh

For headless setup
nano /media/kai/boot/ssh
Placing a file named ssh, without any extension, onto the boot partition of the SD card. When the Pi boots, it looks for the ssh file. If it is found, SSH is enabled, and the file is deleted.

# Setup
sudo apt-get update
sudo apt-get upgrade
sudo apt-get dist-upgrade
# sudo rpi-update

# Disable OS use of UART and Enable UART hardware
set +H
sudo sh -c "echo 'dtoverlay=disable-bt\n' >> /boot/config.txt"
set -H
sudo reboot now
#sudo sh -c "echo 'dtoverlay=disable-bt\ndtoverlay=disable-wifi\n' >> /boot/config.txt"

sudo systemctl disable hciuart
sudo systemctl stop serial-getty@ttyAMA0.service
sudo systemctl disable serial-getty@ttyAMA0.service
sudo systemctl stop serial-getty@ttyS0.service
sudo systemctl disable serial-getty@ttyS0.service
sudo reboot now

# Mount ssd
lsblk -f
sudo mkfs -t ext4 /dev/nvme0n1
# sudo mkfs -t vfat /dev/nvme0n1
sudo mkdir -p /home/pi/ssd
sudo mount -t auto /dev/nvme0n1 /home/pi/ssd
sudo chown pi ssd/
sudo mount /dev/nvme0n1 /home/pi/ssd

# Raspberry camera
sudo gst-launch-1.0 v4l2src device=/dev/video0 ! video/x-raw,width=640,height=480,framerate=20/1 ! x264enc ! matroskamux ! filesink location=/home/pi/ssd/test.mkv
sudo gst-launch-1.0 v4l2src device=/dev/video0 ! video/x-raw,width=1280,height=720,framerate=30/1 ! omxh264enc ! video/x-h264, control-rate=2,bitrate=16650000,insert-sps-pps=true,profile=high ! h264parse ! matroskamux ! filesink location=/home/pi/ssd/test1.mkv
sudo gst-launch-1.0 v4l2src device=/dev/video0 ! video/x-raw,width=1280,height=720,framerate=30/1 ! v4l2h264enc ! h264parse ! matroskamux ! filesink location=/home/pi/ssd/test1.mkv
sudo gst-launch-1.0 v4l2src device=/dev/video0 ! video/x-raw,width=1280,height=720,framerate=30/1 ! videoconvert ! fbdevsink
sudo gst-launch-1.0 libcamerasrc ! video/x-raw,width=1280,height=720,framerate=30/1 ! videoconvert ! fbdevsink
sudo gst-launch-1.0 libcamerasrc ! video/x-raw,format=RGB,width=1280,height=720,framerate=30/1 ! v4l2convert ! v4l2h264enc ! 'video/x-h264,level=(string)4' ! h264parse ! matroskamux ! filesink location="/home/pi/ssd/video/test.mka"

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

# Switch to external antenna.
sudo nano /boot/config.txt
dtparam=ant2
Params:
ant1                    Select antenna 1 (default). CM4 only.
ant2                    Select antenna 2. CM4 only.
noant                   Disable both antennas. CM4 only.

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

# clear bash history
history -c

# outdated
# Raspberry camera
sudo apt-get -y install gstreamer1.0-omx libraspberrypi-dev
git clone --depth 1 https://github.com/thaytan/gst-rpicamsrc.git
cd gst-rpicamsrc
./autogen.sh --prefix=/usr --libdir=/usr/lib/arm-linux-gnueabihf/
make
sudo make install
