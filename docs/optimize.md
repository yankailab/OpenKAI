# Clean-up

Raspberry pi OS

```bash
nano /boot/firmware/config.txt
dtparam=audio=off
```

```bash
systemctl --user stop mpris-proxy.service
systemctl --user disable mpris-proxy.service
systemctl --user mask mpris-proxy.service

sudo systemctl stop bluetooth.service
sudo systemctl disable bluetooth.service

sudo systemctl stop ModemManager
sudo systemctl disable ModemManager
sudo apt-get -y purge modemmanager

sudo systemctl stop hciuart.service
sudo systemctl disable hciuart.service

sudo systemctl disable apt-daily.service
sudo systemctl disable apt-daily.timer
sudo systemctl disable apt-daily-upgrade.timer
sudo systemctl disable apt-daily-upgrade.service
sudo systemctl disable man-db.service

sudo systemctl disable triggerhappy.service
sudo systemctl disable keyboard-setup.service

sudo systemctl disable wpa_supplicant.service
sudo systemctl disable wifi-country.service
sudo systemctl disable avahi-daemon.service
sudo systemctl disable ntp.service
sudo systemctl disable dhcpcd.service
sudo systemctl disable networking.service
sudo systemctl disable ssh.service
```

Ubuntu
```bash
sudo systemctl stop bluetooth.service
sudo systemctl disable bluetooth.service

sudo systemctl stop ModemManager
sudo systemctl disable ModemManager
sudo apt-get -y purge modemmanager

sudo systemctl stop hciuart.service
sudo systemctl disable hciuart.service

sudo systemctl disable apt-daily.service
sudo systemctl disable apt-daily.timer
sudo systemctl disable apt-daily-upgrade.timer
sudo systemctl disable apt-daily-upgrade.service
sudo systemctl disable man-db.service

sudo systemctl disable triggerhappy.service
sudo systemctl disable keyboard-setup.service

sudo systemctl disable wpa_supplicant.service
sudo systemctl disable wifi-country.service
sudo systemctl disable avahi-daemon.service
sudo systemctl disable ntp.service
sudo systemctl disable dhcpcd.service
sudo systemctl disable networking.service
sudo systemctl disable ssh.service


sudo apt-get -y purge whoopsie
sudo apt -y remove --purge libreoffice* yelp thunderbird rhythmbox

```

# clear system logs
```bash
sudo systemctl stop rsyslog
sudo systemctl disable rsyslog.service
sudo systemctl stop systemd-journald.service
sudo systemctl disable systemd-journald.service
sudo systemctl mask systemd-journald.service
sudo truncate /var/log/*.log --size 0
```

# Delete snap
https://www.baeldung.com/linux/snap-remove-disable
```bash
snap --version
snap list
snap remove firefox
snap remove gtk-common-themes
snap remove gnome-3-38-2004
snap remove snapd-desktop-integration
snap remove snap-store
snap remove core20
snap remove bare
snap remove snapd
snap list

sudo systemctl stop snapd
sudo systemctl disable snapd
sudo systemctl mask snapd
sudo apt purge snapd -y
sudo apt-mark hold snapd

rm -rf ~/snap/
```

# Preventing Snap Installation Through the apt Command
```bash
sudo cat <<EOF | sudo tee /etc/apt/preferences.d/nosnap.pref
Package: snapd
Pin: release a=*
Pin-Priority: -10
EOF

rm -rf ~/snap
sudo rm -rf /snap
sudo rm -rf /var/snap
sudo rm -rf /var/lib/snapd
```

# Clear bash history
```bash
history -c && history -w
```

# Ubuntu power mode
```bash
cat /sys/devices/system/cpu/cpu0/cpufreq/scaling_available_governors
cat /sys/devices/system/cpu/cpu0/cpufreq/scaling_governor
echo performance | sudo tee /sys/devices/system/cpu/cpu*/cpufreq/scaling_governor
```

